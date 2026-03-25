// StencilLUTViewExtension.cpp

#include "StencilLUTViewExtension.h"
#include "RenderGraphBuilder.h"
#include "ScenePrivate.h"
#include "SceneRendering.h"
#include "PostProcess/PostProcessing.h"
#include "PostProcess/PostProcessMaterial.h"

DEFINE_LOG_CATEGORY(LogStencilLUT);

FStencilLUTViewExtension::FStencilLUTViewExtension(const FAutoRegister& AutoReg, UStencilMatrixLUTSubsystem* InWorldSubSystem)
	: FSceneViewExtensionBase(AutoReg)
	, WorldSubsystem(InWorldSubSystem)
{
}

void FStencilLUTViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
	// Render-thread, once per family per frame
	UE_LOG(LogTemp, Verbose, TEXT("FStencilLUTViewExtension::BeginRenderViewFamily"));
}

// A simple struct to hold stencil + matrix
struct FStencilAndMatrix
{
	uint8 Stencil;
	FMatrix WorldToLocal;
};

// This function is called on the render thread before post-processing begins for each view
// The texture storing inverse transform matrices is bound on begin play, and this pass will be executed each frame before post-processing
void FStencilLUTViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	if (!IsValid(WorldSubsystem))
	{
		// If we don't have a valid subsystem, nothing to do
		return;
	}
	if (!View.bIsViewInfo)
	{
		UE_LOG(LogStencilLUT, Warning, TEXT("FStencilLUTViewExtension::PrePostProcessPass_RenderThread: View is not FViewInfo"));
		return;
	}
	// The ViewInfo gives us access to PrimitiveVisibilityMap
	const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);
	// The Scene gives us access to primitives (raw 3D objects in the scene) and their associated data
	const FScene* Scene = static_cast<const FScene*>(View.Family->Scene);

	if (!Scene)
	{
		UE_LOG(LogStencilLUT, Warning, TEXT("FStencilLUTViewExtension::PrePostProcessingPass_RenderThread: Invalid Scene"));
		return;
	}

	// Collect visible primitives' proxies (we need custom stencil data)
	TArray<const FPrimitiveSceneProxy*> Proxies;
	// Can store up to 256 entries (stencil values 1-255)
	Proxies.Reserve(256);

	// FSceneBitIterator is not really documented, but it essentially iterates over bits set in a bit array
	// Faster than checking each primitive individually
	for (FSceneSetBitIterator BitIt(ViewInfo.PrimitiveVisibilityMap); BitIt; ++BitIt)
	{
		const int32 PrimIndex = BitIt.GetIndex();
		const FPrimitiveSceneInfo* PrimitiveSceneInfo = Scene->Primitives[PrimIndex];
		if (!PrimitiveSceneInfo) continue;

		const FPrimitiveSceneProxy* Proxy = PrimitiveSceneInfo->Proxy;
		if (!Proxy) continue;

		// Require that it actually writes custom depth
		if (!Proxy->ShouldRenderCustomDepth())
		{
			continue;
		}
		Proxies.Add(Proxy);
	}

	TArray<FStencilAndMatrix> StencilMatrices;
	StencilMatrices.Reserve(Proxies.Num());

	for (const FPrimitiveSceneProxy* Proxy : Proxies)
	{
		// The stencil value assigned to this primitive
		// The subsystem is responsible for assigning stencil values to actors/primitives
		// Here we are simply consuming those values
		uint8 Stencil = Proxy->GetCustomDepthStencilValue();
		if (Stencil == 0)   // treat 0 as “unused”
		{
			continue;
		}

		// Get the local-to-world matrix and invert it to get world-to-local
		const FMatrix LocalToWorld = Proxy->GetLocalToWorld();
		const FMatrix WorldToLocal = LocalToWorld.Inverse();

		FStencilAndMatrix Entry;
		Entry.Stencil = Stencil;
		Entry.WorldToLocal = WorldToLocal;
		StencilMatrices.Add(Entry);
	}

	// Retrieve the LUT texture’s GPU resource for render-thread access.
	//
	// Important distinctions:
	//
	// - UTexture2D is a UObject that exists on the GAME thread.
	//   It handles asset metadata, streaming, GC, editor linkage, etc.
	//
	// - Every UTexture2D owns a Render Resource (FTexture2DResource),
	//   which in turn owns an FRHITexture2D — the ACTUAL GPU memory.
	//
	// - The RHI texture (FRHITexture*) is the only thing that can be safely
	//   accessed and modified on the RENDER THREAD. It is API-agnostic
	//   (D3D12/Vulkan/Metal) and thread-safe for GPU operations.
	//
	// That’s why we query GetTexture2DRHI() instead of touching the UObject
	// directly from here.
	FRHITexture* StencilLUTTexture = WorldSubsystem->GetLUTTextureRHI_RenderThread();
	if (!StencilLUTTexture)
	{
		UE_LOG(LogStencilLUT, Warning, TEXT("FStencilLUTViewExtension::PrePostProcessPass_RenderThread: Invalid StencilLUTTexture"));
		return;
	}

	const uint32 TexWidth = StencilLUTTexture->GetSizeX();
	const uint32 TexHeight = StencilLUTTexture->GetSizeY();

	// RHILockTexture2D gives us a CPU pointer to the texture’s pixel data so we can write/read it, and tells us the row stride.
	// A stride is the number of bytes between the start of one row of pixels and the start of the next row.
	// In our case, each pixel is 16 bytes (4 floats), so the stride will be at least TexWidth * 16 bytes.
	// TLDR: "Hey GPU, imma write some data here, lock it and don't do anything until I unlock"
	uint32 DestStrideBytes = 0;
	void* DestData = RHILockTexture2D(
		StencilLUTTexture,
		/*MipIndex*/ 0,
		RLM_WriteOnly,
		DestStrideBytes,
		/*bLockWithinMiptail*/ false
	);

	uint8* BasePtr = static_cast<uint8*>(DestData);
	const uint32 BytesPerPixel = sizeof(FVector4f); // 4 floats

	// Unreal's FMatrix is a 4x4 row-major transform matrix:
	//
	//   struct FMatrix { float M[4][4]; };  // M[Row][Column]
	//
	// For a typical local→world transform, the rows look like:
	//
	//     // Basis vectors (axes) stored as ROWS, translation in the LAST ROW.
	//     M[0] = [ Xx  Xy  Xz  0 ]   // local X axis in world space
	//     M[1] = [ Yx  Yy  Yz  0 ]   // local Y axis in world space
	//     M[2] = [ Zx  Zy  Zz  0 ]   // local Z axis in world space
	//     M[3] = [ Tx  Ty  Tz  1 ]   // translation (world position)
	//
	// i.e. the full matrix is:
	//
	//     | Xx  Xy  Xz  0 |
	//     | Yx  Yy  Yz  0 |
	//     | Zx  Zy  Zz  0 |
	//     | Tx  Ty  Tz  1 |
	//
	// This is the row-vector convention (p' = p * M), which is effectively the
	// transpose of the column-vector convention taught in many graphics courses,
	// where translation is in the LAST COLUMN and the LAST ROW is [0 0 0 1].
	//
	// In the code below we pack the MATRIX COLUMNS into our LUT texture.
	// For a given Row index R = 0,1,2 we write:
	//
	//     Out[0] = M[0][R];   // column R, row 0
	//     Out[1] = M[1][R];   // column R, row 1
	//     Out[2] = M[2][R];   // column R, row 2
	//     Out[3] = M[3][R];   // column R, row 3
	//
	// So for each stencil ID X, the 3 texels at (X, Y=0..2) store:
	//
	//     (column 0), (column 1), (column 2)
	//
	// which the material later reconstructs into the full 3x4 transform.
	// See the material function "InverseTransformLUT" to see how we sample the data back.

	for (const FStencilAndMatrix& Entry : StencilMatrices)
	{
		const uint32 X = Entry.Stencil;  // column = stencil
		if (X >= TexWidth)
		{
			continue; // safety
		}

		const FMatrix& M = Entry.WorldToLocal;

		for (uint32 Row = 0; Row < 3u; ++Row)
		{
			const uint32 Y = Row;
			if (Y >= TexHeight)
			{
				break;
			}

			uint8* PixelPtr = BasePtr + Y * DestStrideBytes + X * BytesPerPixel;
			float* Out = reinterpret_cast<float*>(PixelPtr);

			Out[0] = M.M[0][Row];
			Out[1] = M.M[1][Row];
			Out[2] = M.M[2][Row];
			Out[3] = M.M[3][Row];
		}
	}

	// Unlock the texture to apply our changes
	// "Hey GPU, the actions I wanted to perform are done, you can now use the updated texture, read and write as you will"
	RHIUnlockTexture2D(StencilLUTTexture, 0, /*bLockWithinMiptail*/ false);
}