#include "DirX12RendererLogic.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "../Utils/CameraMovement.h"


using namespace Wing3D;

bool Wing3D::DirX12RendererLogic::Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig, 
                                        GW::GRAPHICS::GDirectX12Surface _d3d, GW::SYSTEM::GWindow _window, 
                                        GW::INPUT::GInput _ginput, GW::INPUT::GController _gcontroller)
{
    log.Create("renderLogs.txt");
    log.EnableConsoleLogging(true);
    lvlData.LoadLevel("../Assets/GameLevel.txt", "../Assets/Models", log);

    // save a handle to the ECS & game settings
    game = _game;
    gameConfig = _gameConfig;
    d3d = _d3d;
    window = _window;
    ginput = _ginput;
    gcontroller = _gcontroller;

    // get swapchain buffer count
    IDXGISwapChain4* swapChain = nullptr;
    d3d.GetSwapchain4((void**)&swapChain);
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    maxActiveFrames = desc.BufferCount;
    swapChain->Release();
        
    if (SetupShaderVars() == false)
        return false;
    if (SetupGraphics() == false)
        return false;
    if (SetupDrawcalls() == false)
        return false;

    return true;
}

bool Wing3D::DirX12RendererLogic::Activate(bool runSystem)
{
    if (startDraw.is_alive() &&
        updateDraw.is_alive() &&
        completeDraw.is_alive()) {
        if (runSystem) {
            startDraw.enable();
            updateDraw.enable();
            completeDraw.enable();
        }
        else {
            startDraw.disable();
            updateDraw.disable();
            completeDraw.disable();
        }
        return true;
    }
    return false;
}

bool Wing3D::DirX12RendererLogic::Shutdown()
{
    startDraw.destruct();
    updateDraw.destruct();
    completeDraw.destruct();
    return true;
}

void Wing3D::DirX12RendererLogic::SetBackgroundColor(float* bColor)
{
    backgroundColor = bColor;
}

bool Wing3D::DirX12RendererLogic::SetupShaderVars()
{
    InitializeViewMatrix();
    InitializeProjectionMatrix();
    InitializeSceneDataForGPU();

    return true;
}

bool Wing3D::DirX12RendererLogic::SetupGraphics()
{
    InitializeGraphics();

    return true;
}

bool Wing3D::DirX12RendererLogic::SetupDrawcalls()
{
    struct RenderingSystem{};
    game->entity("Rendering System").add<RenderingSystem>();

    startDraw = game->system<RenderingSystem>().kind(flecs::PreUpdate)
        .each([this](flecs::entity e, RenderingSystem& s) {
        //nothing here yet


    });

    updateDraw = game->system<Position, Orientation, Material>().kind(flecs::OnUpdate)
        .each([this](flecs::entity e, Position& p, Orientation& o, Material& m) {
 
        int modelIndex = lvlData.ModelIndices[e.get<Model>()->name];

        GW::MATH::GMATRIXF entM = GW::MATH::GIdentityMatrixF;
        // orient
        GW::MATH::GMatrix::RotateXLocalF(entM, G_DEGREE_TO_RADIAN_F(o.value.x), entM);
        GW::MATH::GMatrix::RotateYLocalF(entM, G_DEGREE_TO_RADIAN_F(o.value.y), entM);
        GW::MATH::GMatrix::RotateZLocalF(entM, G_DEGREE_TO_RADIAN_F(o.value.z), entM);
        // pos
        entM.row4.x = p.value.x;
        entM.row4.y = p.value.y;
        entM.row4.z = p.value.z;
    });

    completeDraw = game->system<RenderingSystem>().kind(flecs::PostUpdate)
        .each([this](flecs::entity e, RenderingSystem& s) {

        GW::MATH::GMATRIXF cameraMatrix;
        GW::MATH::GMatrix::InverseF(viewMatrix, cameraMatrix);
        float aspectRatio;
        d3d.GetAspectRatio(aspectRatio);
        cameraMatrix = CameraMovement::Get().GetCameraMatrixFromInput(cameraMatrix, aspectRatio, window, ginput, gcontroller);
        GW::MATH::GMatrix::InverseF(cameraMatrix, viewMatrix);

        GW::MATH::GMatrix::ProjectionDirectXLHF(G_DEGREE_TO_RADIAN_F(65), aspectRatio, 0.1f, 100, projectionMatrix);
        GW::MATH::GMatrix::MultiplyMatrixF(viewMatrix, projectionMatrix, sceneDataForGPU.viewProjection);
        sceneDataForGPU.camPos = cameraMatrix.row4;

        PipelineHandles curHandles = GetCurrentPipelineHandles();
        SetupPipeline(curHandles);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv;
        D3D12_CPU_DESCRIPTOR_HANDLE dsv;
        if (+d3d.GetCurrentRenderTargetView((void**)&rtv) &&
            +d3d.GetDepthStencilView((void**)&dsv))
        {
            curHandles.commandList->ClearRenderTargetView(rtv, backgroundColor, 0, nullptr);
            curHandles.commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
        }
        UINT curFrame = 0;
        d3d.GetSwapChainBufferIndex(curFrame);
        UpdateTransformsForGPU(curFrame);

        curHandles.commandList->SetGraphicsRoot32BitConstants(0, 32, &sceneDataForGPU, 0);
        curHandles.commandList->SetGraphicsRootShaderResourceView(2, transformStrdBuffer[curFrame]->GetGPUVirtualAddress());
        curHandles.commandList->SetGraphicsRootShaderResourceView(3, materialStrdBuffer[curFrame]->GetGPUVirtualAddress());

        for (int instance = 0; instance < lvlData.levelInstances.size(); instance++)
        {
            int model = lvlData.levelInstances[instance].modelIndex;
            for (int mesh = lvlData.levelModels[model].meshStart;
                mesh < lvlData.levelModels[model].meshStart + lvlData.levelModels[model].meshCount; mesh++)
            {
                meshDataForGPU.materialIndex = mesh;
                meshDataForGPU.transformIndexStart = lvlData.levelInstances[instance].transformStart;
                curHandles.commandList->SetGraphicsRoot32BitConstants(1, 2, &meshDataForGPU, 0);

                curHandles.commandList->DrawIndexedInstanced(lvlData.levelMeshes[mesh].drawInfo.indexCount, lvlData.levelInstances[instance].transformCount,
                    lvlData.levelModels[model].indexStart + lvlData.levelMeshes[mesh].drawInfo.indexOffset, lvlData.levelModels[model].vertexStart, 0);
            }
        }

        curHandles.commandList->Release();
     });

    return true;
}
