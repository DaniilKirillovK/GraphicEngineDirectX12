#include "UIManager.h"
#include <cmath>
#include "CommonData.h"


static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 800;

// Imgui Variables
bool opened = true;

int activeSceneID = 17;

bool isDebug = true;
bool gridIsActive = false;

bool flashlightIsActive = false;

bool isNegative = false;

int tilesCountInt = 1;
int selectedObjectID = 1;
bool isAnimateMaterial = true;
bool isSolid = true;
bool deferredRenderDisplayInfo = false;
bool isParallaxMapping = false;
bool isDeferredRender = false;

bool fpsObjectIsActive = false;

float displacementLevel = 1.0f;

bool isPixelated = false;
int pixelationFactor = 16;

int instancingLevel = 5;

float Obj1Scale = 1.0f;

float Obj1posX = 0.f;
float Obj1posY = 0.f;
float Obj1posZ = 0.f;

float Obj1rotX = 0.f;
float Obj1rotY = 0.f;
float Obj1rotZ = 0.f;


float Obj2posX = 4.f;
float Obj2posY = 0.f;
float Obj2posZ = 0.f;

float Obj2rotX = 0.f;
float Obj2rotY = 0.f;
float Obj2rotZ = 0.f;

float tessFactor = 1.f;

float col1[3] = { 1.0f, 1.0f, 1.0f };
float col2[3] = { 1.0f, 1.0f, 1.0f };
float col3[3] = { 1.0f, 1.0f, 1.0f };

float lightPos1[3] = { 0.0f, 0.0f, 3.0f };
float lightPos2[3] = { 0.0f, 0.0f, 0.0f };
float lightPos3[3] = { 0.0f, 0.0f, -3.0f };

float light1Strength = 0.5f;
float light2Strength = 0.5f;
float light3Strength = 0.5f;

float light1Distance = 1.0f;
float light2Distance = 1.0f;
float light3Distance = 1.0f;


float lightPosSpot1[3] = { 0.0f, 0.0f, 0.0f };
float spotLight1Direction[3] = { 0.0, 0.0f, 0.0f };
float light1SpotStrength = 0.5f;
float light1SpotDistance = 1.0f;
float spotLight1Power = 64.f;
float colSpot1[3] = { 1.0f, 1.0f, 1.0f };

float light2SpotStrength = 0.5f;
float light2SpotDistance = 3.0f;
float spotLight2Power = 64.f;
float colSpot2[3] = { 1.0f, 1.0f, 1.0f };

bool isFrustumCullingScene3 = false;
bool isDisplayingFrustumCullingInfoScene3 = false;
bool isUsingInstancingScene3 = true;
bool isUsingManualLODScene3 = true;
int levelOfDetailsScene3 = 2;

bool isAnimateMaterialScene4 = false;
int tilesCountScene4 = 1;
int filteringModeScene4 = 0;
int addressModeScene4 = 0;

float tessFactorScene5 = 1.f;
bool isSolidScene5 = true;
float displacementLevelScene5 = 1.0f;


int activeParticleSystemScene6 = 1;

bool particles1IsActive = true;
float particleStartColorScene6[3] = { 1.0f, 1.0f, 1.0f };
float particleEndColorScene6[3] = { 0.0f, 0.0f, 0.0f };
float particleStartSizeScene6 = 1.0f;
float particleEndSizeScene6 = 0.5f;

bool particles2IsActive = false;
float particle2StartColorScene6[3] = { 1.0f, 1.0f, 1.0f };
float particle2EndColorScene6[3] = { 0.0f, 0.0f, 0.0f };
float particle2StartSizeScene6 = 1.0f;
float particle2EndSizeScene6 = 0.5f;

int selectedEffectScene8 = 0;
bool isActiveNormalScene8 = true;
bool isActiveGCScene8 = false;
bool isActiveGBScene8 = false;
bool isActiveCAScene8 = false;
bool isActiveVigScene8 = false;
bool isActiveNoiseScene8 = false;
float gammaRatioScene8 = 2.2f;
float textureSizeScene8 = 1000.f;
DirectX::XMFLOAT2 caDistortionScene8(0.05f, 0.05f);
DirectX::XMFLOAT2 caDirectionScene8(1.0f, 0.0f);
bool gbIsHorizontalScene8 = true;
float caIntensityScene8 = 1.0f;
float caPaddingScene8 = 1.0f;
DirectX::XMFLOAT2 vCenterScene8(0.5f, 0.5f);
float vIntensityScene8 = 1.0f;
float vSmoothnessScene8 = 1.0f;
float vRoundnessScene8 = 1.0f;
float nIntensityScene8 = 0.5f;
float nSizeScene8 = 2.f;

int selectedRenderTechScene10 = 2;
bool deferredRenderDisplayInfoScene10 = false;
bool isDebugLayerActiveScene10 = false;
float col1Scene10[3] = { 0.0f, 0.0f, 1.0f };
float col2Scene10[3] = { 0.0f, 1.0f, 0.0f };
float col3Scene10[3] = { 1.0f, 0.0f, 0.0f };
float lightPos1Scene10[3] = { 0.0f, 1.0f, 2.0f };
float lightPos2Scene10[3] = { 0.0f, 1.0f, 0.0f };
float lightPos3Scene10[3] = { 0.0f, 1.0f, -2.0f };
float light1StrengthScene10 = 1.0f;
float light2StrengthScene10 = 1.0f;
float light3StrengthScene10 = 1.0f;
float light1DistanceScene10 = 2.0f;
float light2DistanceScene10 = 2.0f;
float light3DistanceScene10 = 2.0f;
float lightPosSpot1Scene10[3] = { 0.0f, 0.0f, 0.0f };
float spotLight1DirectionScene10[3] = { -2.0f, 1.5f, 0.0f };
float light1SpotStrengthScene10 = 5.0f;
float light1SpotDistanceScene10 = 18.0f;
float spotLight1PowerScene10 = 64.f;
float colSpot1Scene10[3] = { 1.0f, 1.0f, 0.0f };
bool isUsingPositionBufferScene10 = true;

bool isMoreLightActiveScene10 = false;
float moreLightDistanceScene10 = 0.2f;
float moreLightColScene10[3] = { 1.0f, 1.0f, 0.0f };
float moreLightStrengthScene10 = 1.0f;

bool isWireframeScene11 = false;
float tessFactorScene11 = 1.0f;
bool bIsBackCullingScene11 = false;
bool bIsDisplacementAdaptiveTessScene11 = false;
bool bIsDistantAdaptiveTessScene11 = false;
DirectX::XMFLOAT3 decalsPositionScene11[3] =
{
    DirectX::XMFLOAT3(0.f, 1.0f, -2.5f),
    DirectX::XMFLOAT3(-2.5f, 1.0f, 0.0f),
    DirectX::XMFLOAT3(0.f, -1.0f, -2.5f)
};
float decalsDisplacementScaleScene11[3] = { 1.0f, 1.0f, 1.0f };
bool decalsIsActiveScene11[3] = { false, false, false };
float decalsScaleScene11[3] = { 1.0f, 1.0f, 1.0f };

float directionalLightPositionScene7 = 0.f;
bool isTexturedShadowsScene7 = false;
int shadowTextureIDScene7 = 1;
bool isUsingCascadedShadowsScene7 = false;
int shadowSizeIDScene7 = 3;
int shadowFilteringIDScene7 = 2;
bool isVerticalLightScene7 = false;
bool isObjectsActiveScene7 = false;

int lightingIDScene6 = 1;

int PBRShaderScene9 = 1;
bool isRoughnessMetallicDemoScene9 = false;

bool isWireframeScene12 = false;
float tessFactorScene12 = 1.0f;
bool isDistantAdaptiveTessScene12 = true;
float displacementScaleScene12 = 1.0f;

bool isDebugOctreeScene13 = false;
bool isActiveOctreeCullingInfoScene13 = false;
bool isUsingOctreeCullingScene13 = false;

float displacementScaleScene14 = 1.0f;
bool isWireframeScene14 = false;
bool bIsPaintingScene14 = false;
float PaintColorScene14[3] = { 0.0f, 0.0f, 0.0f };

bool isMovingObjectScene15 = false;
int selectedRenderModeScene15 = 0;
bool bIsPausedScene15 = false;
bool drawScene15 = false;

float scene16SunColor[3] = { 1.0f, 1.0f, 1.0f };
float scene16ScaterringIntensity = 20.f;
float scene16RayleiCoef[3] = { 6.95e-6f, 1.18e-5f, 2.44e-5f };
float scene16MieCoef = { 2.1e-5f };
DirectX::XMFLOAT3 scene16SunPosition = { 0.0f, -sin(2.f * 3.1415926535f), -cos(2.f * 3.1415926535f) };

bool bIsUsingTexturingScene18 = true;
int vertexAnimationModeScene18 = 0;
bool bIsBloomActiveScene19 = false;
int bloomRadiusScene19 = 12;


void UIManager::RenderUI(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    std::vector<std::unique_ptr<RenderItem>>& allRitems,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvHeap,
    UINT cbvSrvDescriptorSize,
    std::unordered_map<std::string, std::unique_ptr<Material>>& materials)
{
    ImVec2 infoPanelSize = ImVec2(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4);
    ImVec2 infoPanelPos = ImVec2(0, 0);
    ImGui::SetNextWindowPos(infoPanelPos);
    ImGui::SetNextWindowSize(infoPanelSize);
    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::Begin("Info Panel", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("Scene 1: Sandbox");
        ImGui::Text("Scene 2: None");
        ImGui::Text("Scene 3: Instancing/frumstum culling scene");
        ImGui::Text("Scene 4: Texture animation & Tiling scene");
        ImGui::Text("Scene 5: Tessellation scene (old)");
        ImGui::Text("Scene 6: Particles scene");
        ImGui::Text("Scene 7: Shadows scene");
        ImGui::Text("Scene 8: Post-processing scene");
        ImGui::Text("Scene 9: PBR scene");
        ImGui::Text("Scene 10: Rendering Techniques");
        ImGui::Text("Scene 11: Decals Tessellation");
        ImGui::Text("Scene 12: Terrain with Height map");
        ImGui::Text("Scene 13: OcTree");
        ImGui::Text("Scene 14: Terrain");
        ImGui::Text("Scene 15: TAA");
        ImGui::Text("Scene 16: Atmosphere");
        ImGui::Text("Scene 17: Cube Marching");
        ImGui::Text("Scene 18: Mesh Pipeline");
        ImGui::Text("Scene 19: Bloom");
    } ImGui::End();

    ImVec2 scenePanelSize = ImVec2(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 10);
    ImVec2 scenePanelPos = ImVec2(WINDOW_WIDTH / 2 - scenePanelSize.x / 2, WINDOW_HEIGHT - scenePanelSize.y);
    ImGui::SetNextWindowPos(scenePanelPos);
    ImGui::SetNextWindowSize(scenePanelSize);
    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::Begin("Scene Selector", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
    {
        if (ImGui::BeginTable("Scenes", 8))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Button("Scene 1", ImVec2(80, 40)))
            {
                activeSceneID = 1;
            }

            ImGui::TableSetColumnIndex(1);
            if (ImGui::Button("Scene 2", ImVec2(80, 40)))
            {
                activeSceneID = 2;
            }

            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button("Scene 3", ImVec2(80, 40)))
            {
                activeSceneID = 3;
            }

            ImGui::TableSetColumnIndex(3);
            if (ImGui::Button("Scene 4", ImVec2(80, 40)))
            {
                activeSceneID = 4;
            }

            ImGui::TableSetColumnIndex(4);
            if (ImGui::Button("Scene 5", ImVec2(80, 40)))
            {
                activeSceneID = 5;
            }

            ImGui::TableSetColumnIndex(5);
            if (ImGui::Button("Scene 6", ImVec2(80, 40)))
            {
                activeSceneID = 6;
            }

            ImGui::TableSetColumnIndex(6);
            if (ImGui::Button("Scene 7", ImVec2(80, 40)))
            {
                activeSceneID = 7;
            }

            ImGui::TableSetColumnIndex(7);
            if (ImGui::Button("Scene 8", ImVec2(80, 40)))
            {
                activeSceneID = 8;
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Button("Scene 9", ImVec2(80, 40)))
            {
                activeSceneID = 9;
            }

            ImGui::TableSetColumnIndex(1);
            if (ImGui::Button("Scene 10", ImVec2(80, 40)))
            {
                activeSceneID = 10;
            }
            ImGui::TableSetColumnIndex(2);

            if (ImGui::Button("Scene 11", ImVec2(80, 40)))
            {
                activeSceneID = 11;
            }
            ImGui::TableSetColumnIndex(3);

            if (ImGui::Button("Scene 12", ImVec2(80, 40)))
            {
                activeSceneID = 12;
            }
            ImGui::TableSetColumnIndex(4);

            if (ImGui::Button("Scene 13", ImVec2(80, 40)))
            {
                activeSceneID = 13;
            }
            ImGui::TableSetColumnIndex(5);

            if (ImGui::Button("Scene 14", ImVec2(80, 40)))
            {
                activeSceneID = 14;
            }
            ImGui::TableSetColumnIndex(6);

            if (ImGui::Button("Scene 15", ImVec2(80, 40)))
            {
                activeSceneID = 15;
            }
            ImGui::TableSetColumnIndex(7);

            if (ImGui::Button("Scene 16", ImVec2(80, 40)))
            {
                activeSceneID = 16;
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Button("Scene 17", ImVec2(80, 40)))
            {
                activeSceneID = 17;
            }

            ImGui::TableSetColumnIndex(1);
            if (ImGui::Button("Scene 18", ImVec2(80, 40)))
            {
                activeSceneID = 18;
            }

            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button("Scene 19", ImVec2(80, 40)))
            {
                activeSceneID = 19;
            }

            ImGui::EndTable();
        }
    } ImGui::End();

    ImVec2 size = ImVec2(WINDOW_WIDTH / 5, WINDOW_HEIGHT / 4 * 3);
    ImVec2 pos = ImVec2(WINDOW_WIDTH - size.x, 0);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::Begin("Configuration", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

        if (activeSceneID == 1)
        {
            ImGui::Text("");
            ImGui::Text("Deferred Render");
            ImGui::Checkbox("Deferred Render Info", &deferredRenderDisplayInfo);
        }

        if (activeSceneID == 1)
        {
            ImGui::Text("");
            ImGui::Checkbox("Debug Layer", &isDebug);
            ImGui::Checkbox("Debug Grid", &gridIsActive);
            ImGui::Text("");

            ImGui::Checkbox("FPS Object", &fpsObjectIsActive);
            ImGui::Text("");

            ImGui::Text("Flashlight");
            ImGui::Checkbox("Is Active", &flashlightIsActive);
            ImGui::SliderFloat("Distance Flashlight", &light2SpotDistance, 0.0f, 20.0f);
            ImGui::SliderFloat("Strength Flashlight", &light2SpotStrength, 0.0f, 5.0f);
            ImGui::ColorPicker3("Color Flashlight", colSpot2, ImGuiColorEditFlags_NoAlpha);

            ImGui::Text("");
            ImGui::Text("Parallax Mapping");
            ImGui::Checkbox("Parallax Mapping", &isParallaxMapping);

            ImGui::Text("");
            ImGui::Text("Tessellation & Displacement");
            ImGui::Checkbox("Solid Mode", &isSolid);
            ImGui::SliderFloat("Tessellation Factor", &tessFactor, 1.f, 64.f);
            ImGui::SliderFloat("Displacement Level", &displacementLevel, 0.f, 5.f);

            ImGui::Text("");
            ImGui::Text("Effects");
            ImGui::Checkbox("Negative", &isNegative);
            ImGui::Checkbox("Pixelation Shader", &isPixelated);
            ImGui::SliderInt("Pixelated Factor", &pixelationFactor, 16.f, 128.f);

            ImGui::Text("");
            ImGui::Text("Instancing");
            ImGui::SliderInt("Instancing Level", &instancingLevel, 1.f, 30.f);

            ImGui::Text("");
            if (ImGui::Button("Object 1", ImVec2(100, 40)))
            {
                if (selectedObjectID == 1)
                    selectedObjectID = 0;
                else selectedObjectID = 1;
            }
            if (ImGui::Button("Object 2", ImVec2(100, 40)))
            {
                if (selectedObjectID == 2)
                    selectedObjectID = 0;
                else selectedObjectID = 2;
            }

            allRitems[0]->NumFramesDirty = 1;
            allRitems[1]->NumFramesDirty = 1;
            allRitems[2]->NumFramesDirty = 1;
            allRitems[4]->NumFramesDirty = 1;
            allRitems[5]->NumFramesDirty = 1;
            allRitems[6]->NumFramesDirty = 1;
            allRitems[9]->NumFramesDirty = 1;
        }
        else if (activeSceneID == 2)
        {
            ImGui::Text("");
        }
        else if (activeSceneID == 3)
        {
            ImGui::Text("");
            ImGui::Text("Frustum Culling");
            ImGui::Checkbox("Frustum Culling Active", &isFrustumCullingScene3);
            ImGui::Checkbox("Frustum Culling Info", &isDisplayingFrustumCullingInfoScene3);

            ImGui::Text("");
            ImGui::Text("Instancing");
            ImGui::Checkbox("Instancing Active", &isUsingInstancingScene3);

            ImGui::Text("");
            ImGui::Text("LOD");
            ImGui::Checkbox("Manual LOD", &isUsingManualLODScene3);
            if (isUsingManualLODScene3)
                ImGui::SliderInt("Manual Level Of Details", &levelOfDetailsScene3, 0, 3);

            allRitems[763]->NumFramesDirty = 1;
            allRitems[764]->NumFramesDirty = 1;
            allRitems[765]->NumFramesDirty = 1;
            allRitems[766]->NumFramesDirty = 1;
        }
        else if (activeSceneID == 4)
        {
            ImGui::Checkbox("Animate Material", &isAnimateMaterialScene4);
            ImGui::SliderInt("Tiles Count", &tilesCountScene4, 1, 6);
            ImGui::Text("Filtering Mode");
            ImGui::RadioButton("Point", &filteringModeScene4, 0);
            ImGui::RadioButton("Linear", &filteringModeScene4, 1);
            ImGui::RadioButton("Anisotrophic", &filteringModeScene4, 2);
            if (filteringModeScene4 == 0)
            {
                ImGui::Text("Address Mode");
                ImGui::RadioButton("Wrap", &addressModeScene4, 0);
                ImGui::RadioButton("Clamp", &addressModeScene4, 1);
                ImGui::RadioButton("Border", &addressModeScene4, 2);
                ImGui::RadioButton("Mirror", &addressModeScene4, 3);
                ImGui::RadioButton("Mirror Once", &addressModeScene4, 4);
            }
            allRitems[743]->NumFramesDirty = 1;
            allRitems[744]->NumFramesDirty = 1;
        }
        else if (activeSceneID == 5)
        {
            ImGui::Text("");
            ImGui::Text("Tessellation & Displacement");
            ImGui::Checkbox("Solid Mode", &isSolidScene5);
            ImGui::SliderFloat("Tessellation Factor", &tessFactorScene5, 1.f, 64.f);
            ImGui::SliderFloat("Displacement Level", &displacementLevelScene5, 0.f, 5.f);
            allRitems[745]->NumFramesDirty = 1;
        }
        else if (activeSceneID == 6)
        {
            ImGui::Text("");
            ImGui::Text("Particles");
            if (ImGui::BeginTable("Systems", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if (ImGui::Button("System 1", ImVec2(80, 40)))
                {
                    activeParticleSystemScene6 = 1;
                }

                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button("System 2", ImVec2(80, 40)))
                {
                    activeParticleSystemScene6 = 2;
                }

                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("System 3", ImVec2(80, 40)))
                {
                    activeParticleSystemScene6 = 3;
                }

                ImGui::EndTable();
            }

            if (activeParticleSystemScene6 == 1)
            {
                ImGui::Text("");
                ImGui::Checkbox("Particles", &particles1IsActive);
                ImGui::Text("Particle System Settings");
                ImGui::SliderFloat("Particle Start Size", &particleStartSizeScene6, 0.5f, 8.f);
                ImGui::SliderFloat("Particle End Size", &particleEndSizeScene6, 0.0f, 0.5f);
                ImGui::ColorPicker3("Particle Start Color", particleStartColorScene6, ImGuiColorEditFlags_NoAlpha);
                ImGui::ColorPicker3("Particle End Color", particleEndColorScene6, ImGuiColorEditFlags_NoAlpha);
            }
            else if (activeParticleSystemScene6 == 2)
            {
                ImGui::Text("");
                ImGui::Checkbox("Particles", &particles2IsActive);
                ImGui::Text("Particle System Settings");
                ImGui::SliderFloat("Particle Start Size", &particle2StartSizeScene6, 0.5f, 8.f);
                ImGui::SliderFloat("Particle End Size", &particle2EndSizeScene6, 0.0f, 0.5f);
                ImGui::ColorPicker3("Particle Start Color", particle2StartColorScene6, ImGuiColorEditFlags_NoAlpha);
                ImGui::ColorPicker3("Particle End Color", particle2EndColorScene6, ImGuiColorEditFlags_NoAlpha);
            }
            else if (activeParticleSystemScene6 == 3)
            {
                ImGui::Text("");
                ImGui::Text("Lighting Technique");
                ImGui::RadioButton("Vertex Shader", &lightingIDScene6, 1);
                ImGui::RadioButton("Domain Shader", &lightingIDScene6, 2);
            }
        }
        else if (activeSceneID == 7)
        {
            ImGui::Text("");
            ImGui::Text("Directional Light");
            ImGui::Checkbox("Vertical Light", &isVerticalLightScene7);
            if (!isVerticalLightScene7)
                ImGui::SliderFloat("Position", &directionalLightPositionScene7, 0.0f, 10.0f);

            ImGui::Text("");
            ImGui::Text("Shadow Texturing");
            ImGui::Checkbox("Using Shadows Texturing", &isTexturedShadowsScene7);
            if (isTexturedShadowsScene7)
            {
                ImGui::RadioButton("Texture 1", &shadowTextureIDScene7, 1);
                ImGui::RadioButton("Texture 2", &shadowTextureIDScene7, 2);
                ImGui::RadioButton("Texture 3", &shadowTextureIDScene7, 3);
            }

            ImGui::Text("");
            ImGui::Text("Cascaded Shadows");
            ImGui::Checkbox("Using Cascaded Shadows", &isUsingCascadedShadowsScene7);
            if (!isUsingCascadedShadowsScene7)
            {
                ImGui::Text("Shadow Map Size");
                ImGui::RadioButton("256", &shadowSizeIDScene7, 0);
                ImGui::RadioButton("512", &shadowSizeIDScene7, 1);
                ImGui::RadioButton("1024", &shadowSizeIDScene7, 2);
                ImGui::RadioButton("2048", &shadowSizeIDScene7, 3);
            }

            ImGui::Text("");
            ImGui::Text("Shadows Filtering");
            ImGui::RadioButton("Point", &shadowFilteringIDScene7, 0);
            ImGui::RadioButton("Linear", &shadowFilteringIDScene7, 1);
            ImGui::RadioButton("Anisotropic", &shadowFilteringIDScene7, 2);

            ImGui::Text("");
            ImGui::Checkbox("Objects is active", &isObjectsActiveScene7);

            allRitems[755]->NumFramesDirty = 1;
            allRitems[756]->NumFramesDirty = 1;
            allRitems[757]->NumFramesDirty = 1;
        }
        else if (activeSceneID == 8)
        {
            ImGui::Text("");
            ImGui::Text("Post Processing Effects");
            ImGui::RadioButton("Default", &selectedEffectScene8, 0);
            ImGui::RadioButton("Gamma Correction", &selectedEffectScene8, 1);
            ImGui::RadioButton("Gaussian Blur", &selectedEffectScene8, 2);
            ImGui::RadioButton("Chromatic Aberration", &selectedEffectScene8, 3);
            ImGui::RadioButton("Vignette", &selectedEffectScene8, 4);
            ImGui::RadioButton("Noise", &selectedEffectScene8, 5);

            isActiveNormalScene8 = false;
            isActiveGCScene8 = false;
            isActiveGBScene8 = false;
            isActiveCAScene8 = false;
            isActiveVigScene8 = false;
            isActiveNoiseScene8 = false;
            if (selectedEffectScene8 == 0)
            {
                isActiveNormalScene8 = true;
            }
            else if (selectedEffectScene8 == 1)
            {
                isActiveGCScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::SliderFloat("Gamma Ratio", &gammaRatioScene8, 0.2f, 5.0f);
            }
            else if (selectedEffectScene8 == 2)
            {
                isActiveGBScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::Checkbox("Horizontal/Vertical", &gbIsHorizontalScene8);
            }
            else if (selectedEffectScene8 == 3)
            {
                isActiveCAScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::SliderFloat("DistortionX", &caDistortionScene8.x, 0.0f, 0.1f);
                ImGui::SliderFloat("DistortionY", &caDistortionScene8.y, 0.0f, 0.1f);
                ImGui::SliderFloat("Direction", &caDirectionScene8.x, 0.0f, 1.0f);
                caDirectionScene8.y = sqrt(1.f - caDirectionScene8.x * caDirectionScene8.x);
                ImGui::SliderFloat("Intensity", &caIntensityScene8, 0.0f, 2.0f);
            }
            else if (selectedEffectScene8 == 4)
            {
                isActiveVigScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::SliderFloat("Intensity", &vIntensityScene8, 0.5f, 1.5f);
                ImGui::SliderFloat("Smoothness", &vSmoothnessScene8, 0.3f, 1.0f);
                ImGui::SliderFloat("Roundness", &vRoundnessScene8, 0.5f, 1.0f);
            }
            else if (selectedEffectScene8 == 5)
            {
                isActiveNoiseScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::SliderFloat("Intensity", &nIntensityScene8, 0.0f, 3.f);
                ImGui::SliderFloat("Size", &nSizeScene8, 0.5f, 5.f);
            }
        }
        else if (activeSceneID == 9)
        {
            ImGui::RadioButton("Standart Render", &PBRShaderScene9, 0);
            ImGui::RadioButton("PBR", &PBRShaderScene9, 1);
            ImGui::RadioButton("IBL", &PBRShaderScene9, 2);

            ImGui::Text("");
            ImGui::Checkbox("RM Demo", &isRoughnessMetallicDemoScene9);

            allRitems[758]->NumFramesDirty = 1;
            allRitems[759]->NumFramesDirty = 1;
            allRitems[760]->NumFramesDirty = 1;
            allRitems[761]->NumFramesDirty = 1;
            allRitems[762]->NumFramesDirty = 1;
            materials["PBR1Material"]->NumFramesDirty = gNumFrameResources;
            materials["PBR2Material"]->NumFramesDirty = gNumFrameResources;
        }
        else if (activeSceneID == 10)
        {
            ImGui::Text("");
            ImGui::Text("Rendering Techniques");
            ImGui::RadioButton("Forward", &selectedRenderTechScene10, 0);
            ImGui::RadioButton("Deffered", &selectedRenderTechScene10, 1);
            ImGui::RadioButton("Deffered + Light Volumes", &selectedRenderTechScene10, 2);

            if (selectedRenderTechScene10 == 1 || selectedRenderTechScene10 == 2)
            {
                ImGui::Text("");
                ImGui::Text("Other settings");
                ImGui::Checkbox("Deffered Render Info", &deferredRenderDisplayInfoScene10);
            }
            if (selectedRenderTechScene10 == 1)
            {
                ImGui::Checkbox("Using Position Buffer", &isUsingPositionBufferScene10);
            }

            ImGui::Text("");
            ImGui::Checkbox("Debug Layer", &isDebugLayerActiveScene10);

            ImGui::Text("");
            ImGui::Checkbox("More Light", &isMoreLightActiveScene10);
            if (isMoreLightActiveScene10)
            {
                ImGui::SliderFloat("Light Distance", &moreLightDistanceScene10, 0.0f, 3.0f);
                ImGui::SliderFloat("Light Strength", &moreLightStrengthScene10, 0.0f, 5.0f);
                ImGui::ColorPicker3("Color", moreLightColScene10, ImGuiColorEditFlags_NoAlpha);
            }

            allRitems[2]->NumFramesDirty = gNumFrameResources;
            allRitems[751]->NumFramesDirty = gNumFrameResources;
            allRitems[752]->NumFramesDirty = gNumFrameResources;
            allRitems[753]->NumFramesDirty = gNumFrameResources;
            allRitems[754]->NumFramesDirty = gNumFrameResources;
            allRitems[769]->NumFramesDirty = gNumFrameResources;
        }
        else if (activeSceneID == 11)
        {
            ImGui::Text("");
            ImGui::Checkbox("Wireframe", &isWireframeScene11);
            ImGui::Checkbox("Back Face Culling", &bIsBackCullingScene11);
            ImGui::Checkbox("Distance Adaptive Tess", &bIsDistantAdaptiveTessScene11);
            ImGui::Checkbox("Displacement Adaptive Tess", &bIsDisplacementAdaptiveTessScene11);
            ImGui::SliderFloat("Tessellation Factor", &tessFactorScene11, 1.0f, 64.0f);
            ImGui::Text("");
            ImGui::Text("Decals settings");
            ImGui::Text("Decal 1");
            ImGui::Checkbox("Is Active 1", &decalsIsActiveScene11[0]);
            if (decalsIsActiveScene11[0])
            {
                ImGui::SliderFloat("Displacement Scale 1", &decalsDisplacementScaleScene11[0], 0.0f, 2.0f);
                ImGui::SliderFloat("Decal Scale 1", &decalsScaleScene11[0], 0.5f, 3.0f);
            }
            ImGui::Text("Decal 2");
            ImGui::Checkbox("Is Active 2", &decalsIsActiveScene11[1]);
            if (decalsIsActiveScene11[1])
            {
                ImGui::SliderFloat("Displacement Scale 2", &decalsDisplacementScaleScene11[1], 0.0f, 2.0f);
                ImGui::SliderFloat("Decal Scale 2", &decalsScaleScene11[1], 0.5f, 3.0f);
            }
            ImGui::Text("Decal 3");
            ImGui::Checkbox("Is Active 3", &decalsIsActiveScene11[2]);
            if (decalsIsActiveScene11[2])
            {
                ImGui::SliderFloat("Displacement Scale 3", &decalsDisplacementScaleScene11[2], 0.0f, 2.0f);
                ImGui::SliderFloat("Decal Scale 3", &decalsScaleScene11[2], 0.5f, 3.0f);
            }
            allRitems[2]->NumFramesDirty = 1;
        }
        else if (activeSceneID == 12)
        {
            ImGui::Text("");
            ImGui::Checkbox("Wireframe", &isWireframeScene12);
            ImGui::Checkbox("Distance Adaptive Tess", &isDistantAdaptiveTessScene12);
            ImGui::SliderFloat("Tessellation Factor", &tessFactorScene12, 1.0f, 64.0f);
            ImGui::SliderFloat("Displacement Scale", &displacementScaleScene12, 0.0f, 10.0f);
        }
        else if (activeSceneID == 13)
        {
            ImGui::Text("");
            ImGui::Checkbox("Debug Octree", &isDebugOctreeScene13);
            ImGui::Checkbox("Octree Culling Info", &isActiveOctreeCullingInfoScene13);
        }
        else if (activeSceneID == 14)
        {
            ImGui::Text("");
            ImGui::Checkbox("Wireframe", &isWireframeScene14);
            ImGui::SliderFloat("Displacement Scale", &displacementScaleScene14, 0.0f, 150.0f);

            ImGui::Text("");
            ImGui::Checkbox("Painting", &bIsPaintingScene14);
            ImGui::ColorPicker3("Paint Color", PaintColorScene14, ImGuiColorEditFlags_NoAlpha);

            CommonData::PaintColor.x = PaintColorScene14[0];
            CommonData::PaintColor.y = PaintColorScene14[1];
            CommonData::PaintColor.z = PaintColorScene14[2];
        }
        else if (activeSceneID == 15)
        {
            ImGui::Text("");
            ImGui::Checkbox("Move object", &isMovingObjectScene15);

            ImGui::Text("");
            ImGui::Text("Render Mode");
            ImGui::RadioButton("Common", &selectedRenderModeScene15, 0);
            ImGui::RadioButton("TAA", &selectedRenderModeScene15, 1);
            ImGui::Text("");
            ImGui::Checkbox("Paused", &bIsPausedScene15);
            if (bIsPausedScene15)
            {
                if (ImGui::Button("Next Step", ImVec2(80, 40)))
                {
                    drawScene15 = true;
                }
            }

            allRitems[774]->NumFramesDirty = gNumFrameResources;
        }
        else if (activeSceneID == 16)
        {
            ImGui::Text("");
            ImGui::SliderFloat("Scaterring Intensity", &scene16ScaterringIntensity, 0.0f, 30.f);

            ImGui::Text("");
            ImGui::SliderFloat("Mie Coef", &scene16MieCoef, 1.0e-7f, 1.0e-5f, "%.7f");
            ImGui::Text("Rayleigh Coef");
            ImGui::SliderFloat("Rayleigh R", &scene16RayleiCoef[0], 1.0e-6f, 4.0e-5f, "%.6f");
            ImGui::SliderFloat("Rayleigh G", &scene16RayleiCoef[1], 1.0e-6f, 4.0e-5f, "%.6f");
            ImGui::SliderFloat("Rayleigh B", &scene16RayleiCoef[2], 1.0e-6f, 4.0e-5f, "%.6f");

            if (ImGui::BeginTable("Sun Position", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("X");

                ImGui::TableSetColumnIndex(0);
                ImGui::SliderFloat("##SunPosX", &scene16SunPosition.x, -1.0f, 1.0f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::SliderFloat("##SunPosY", &scene16SunPosition.y, 0.0f, 1.0f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::SliderFloat("##SunPosZ", &scene16SunPosition.z, -1.0f, 1.0f);
                ImGui::EndTable();
            }
        }
        else if (activeSceneID == 17)
        {
            ImGui::Text("");
        }
        else if (activeSceneID == 18)
        {
            ImGui::Text("");
            ImGui::Checkbox("Texturing", &bIsUsingTexturingScene18);
            ImGui::Text("");
            ImGui::Text("Vertex Animation Mode");
            ImGui::RadioButton("None", &vertexAnimationModeScene18, 0);
            ImGui::RadioButton("Sync", &vertexAnimationModeScene18, 1);
            ImGui::RadioButton("Chaos", &vertexAnimationModeScene18, 2);
        }
        else if (activeSceneID == 19)
        {
            ImGui::Text("");
            ImGui::Checkbox("Bloom", &bIsBloomActiveScene19);
            ImGui::Text("Bloom Radius");
            ImGui::SliderInt("##BloomRadius", &bloomRadiusScene19, 1, 19);

            GeometryManager::mAllRitems[776]->NumFramesDirty = gNumFrameResources;
            GeometryManager::mAllRitems[777]->NumFramesDirty = gNumFrameResources;
            GeometryManager::mAllRitems[778]->NumFramesDirty = gNumFrameResources;
        }
    } ImGui::End();

    if (deferredRenderDisplayInfo && activeSceneID <= 2)
    {
        ImVec2 size2 = ImVec2(WINDOW_WIDTH / 5, 800);
        ImVec2 pos2 = ImVec2(0, 0);

        ImGui::SetNextWindowPos(pos2);
        ImGui::SetNextWindowSize(size2);
        ImGui::SetNextWindowBgAlpha(0.5f);
        if (ImGui::Begin("Deferred Render Info", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(srvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(11, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
        } ImGui::End();
    }

    if (deferredRenderDisplayInfoScene10 && activeSceneID == 10 && selectedRenderTechScene10 != 0)
    {
        ImVec2 size2 = ImVec2(WINDOW_WIDTH / 5, 800);
        ImVec2 pos2 = ImVec2(0, 0);

        ImGui::SetNextWindowPos(pos2);
        ImGui::SetNextWindowSize(size2);
        ImGui::SetNextWindowBgAlpha(0.5f);
        if (ImGui::Begin("Deferred Render Info", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(srvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(11, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, cbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
        } ImGui::End();
    }

    if (activeSceneID == 1)
    {
        ImVec2 lightPanelSize = ImVec2(350.f, 350.f);
        ImGui::SetNextWindowSize(lightPanelSize);
        if (ImGui::Begin("Point Light Config", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text("Point Light 1");
            if (ImGui::BeginTable("PointLight1", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##PointLightPos1X", &lightPos1[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##PointLightPos1Y", &lightPos1[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##PointLightPos1Z", &lightPos1[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("Distance 1", &light1Distance, 0.0f, 2.0f);
            ImGui::SliderFloat("Strength 1", &light1Strength, 0.0f, 1.0f);
            ImGui::ColorPicker3("Color 1", col1, ImGuiColorEditFlags_NoAlpha);

            ImGui::Text("");
            ImGui::Text("Point Light 2");
            if (ImGui::BeginTable("PointLight2", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##PointLightPos2X", &lightPos2[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##PointLightPos2Y", &lightPos2[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##PointLightPos2Z", &lightPos2[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("Distance 2", &light2Distance, 0.0f, 2.0f);
            ImGui::SliderFloat("Strength 2", &light2Strength, 0.0f, 1.0f);
            ImGui::ColorPicker3("Color 2", col2, ImGuiColorEditFlags_NoAlpha);

            ImGui::Text("");
            ImGui::Text("Point Light 3");
            if (ImGui::BeginTable("PointLight3", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##PointLightPos3X", &lightPos3[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##PointLightPos3Y", &lightPos3[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##PointLightPos3Z", &lightPos3[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("Distance 3", &light3Distance, 0.0f, 2.0f);
            ImGui::SliderFloat("Strength 3", &light3Strength, 0.0f, 1.0f);
            ImGui::ColorPicker3("Color 3", col3, ImGuiColorEditFlags_NoAlpha);

        } ImGui::End();

        ImVec2 spotLightPanelSize = ImVec2(350.f, 350.f);
        ImVec2 spotLightPanelPos = ImVec2(100.f, 200.f);
        ImGui::SetNextWindowSize(spotLightPanelSize);
        ImGui::SetNextWindowPos(spotLightPanelPos);
        if (ImGui::Begin("Spot Light Config", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text("Spot Light 1");
            if (ImGui::BeginTable("SpotLight1", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##SpotLightPos1X", &lightPosSpot1[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##SpotLightPos1Y", &lightPosSpot1[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##SpotLightPos1Z", &lightPosSpot1[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("SpotLightDir1X", &spotLight1Direction[0], -DirectX::XM_PI, DirectX::XM_PI);
            ImGui::SliderFloat("SpotLightDir1Y", &spotLight1Direction[1], -DirectX::XM_PI, DirectX::XM_PI);
            ImGui::SliderFloat("SpotLightDir1Z", &spotLight1Direction[2], -DirectX::XM_PI, DirectX::XM_PI);

            ImGui::SliderFloat("Spot Distance 1", &light1SpotDistance, 0.0f, 20.0f);
            ImGui::SliderFloat("Spot Strength 1", &light1SpotStrength, 0.0f, 10.0f);
            ImGui::ColorPicker3("Spot Color 1", colSpot1, ImGuiColorEditFlags_NoAlpha);

        } ImGui::End();

        // Selected object info
        {
            ImVec2 size3 = ImVec2(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4);
            ImVec2 pos3 = ImVec2(WINDOW_WIDTH / 4 * 3, WINDOW_HEIGHT / 4 * 3);

            ImGui::SetNextWindowPos(pos3);
            ImGui::SetNextWindowSize(size3);
            ImGui::SetNextWindowBgAlpha(0.5f);
            if (ImGui::Begin("Object Settings", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
            {
                switch (selectedObjectID)
                {
                case 1:
                    ImGui::Text("Object 1");
                    if (ImGui::BeginTable("Object1", 3))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Pos X");

                        ImGui::TableSetColumnIndex(0);
                        ImGui::InputFloat("##PosXObj1", &Obj1posX, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Pos Y");

                        ImGui::TableSetColumnIndex(1);
                        ImGui::InputFloat("##PosYObj1", &Obj1posY, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Pos Z");

                        ImGui::TableSetColumnIndex(2);
                        ImGui::InputFloat("##PosZObj1", &Obj1posZ, 0.1f, 0.1f);
                        ImGui::EndTable();
                    }
                    if (ImGui::BeginTable("Object1_1", 3))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Rot X");

                        ImGui::TableSetColumnIndex(0);
                        ImGui::InputFloat("##RotXObj1", &Obj1rotX, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Rot Y");

                        ImGui::TableSetColumnIndex(1);
                        ImGui::InputFloat("##RotYObj1", &Obj1rotY, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Rot Z");

                        ImGui::TableSetColumnIndex(2);
                        ImGui::InputFloat("##RotZObj1", &Obj1rotZ, 0.1f, 0.1f);
                        ImGui::EndTable();
                    }
                    ImGui::SliderFloat("Scale", &Obj1Scale, 0.1f, 5.0f);
                    ImGui::Checkbox("Animate Material", &isAnimateMaterial);
                    break;

                case 2:
                    ImGui::Text("Object 2");
                    if (ImGui::BeginTable("Object2", 3))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Pos X");

                        ImGui::TableSetColumnIndex(0);
                        ImGui::InputFloat("##PosXObj2", &Obj2posX, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Pos Y");

                        ImGui::TableSetColumnIndex(1);
                        ImGui::InputFloat("##PosYObj2", &Obj2posY, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Pos Z");

                        ImGui::TableSetColumnIndex(2);
                        ImGui::InputFloat("##PosZObj2", &Obj2posZ, 0.1f, 0.1f);
                        ImGui::EndTable();
                    }
                    if (ImGui::BeginTable("Object2_1", 3))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Rot X");

                        ImGui::TableSetColumnIndex(0);
                        ImGui::InputFloat("##RotXObj2", &Obj2rotX, 1.f, 1.f);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Rot Y");

                        ImGui::TableSetColumnIndex(1);
                        ImGui::InputFloat("##RotYObj2", &Obj2rotY, 1.f, 1.f);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Rot Z");

                        ImGui::TableSetColumnIndex(2);
                        ImGui::InputFloat("##RotZObj2", &Obj2rotZ, 1.f, 1.f);
                        ImGui::EndTable();
                    }
                    ImGui::SliderInt("Tiles Count", &tilesCountInt, 1, 6);
                    break;
                }
            } ImGui::End();
        }
    }
    else if (activeSceneID == 2)
    {

    }
    else if (activeSceneID == 3)
    {
        if (isDisplayingFrustumCullingInfoScene3 && isUsingInstancingScene3)
        {
            ImVec2 size = ImVec2(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 3);
            ImVec2 pos = ImVec2(WINDOW_WIDTH / 2 - size.x / 2, 0);

            ImGui::SetNextWindowPos(pos);
            ImGui::SetNextWindowSize(size);
            ImGui::SetNextWindowBgAlpha(0.5f);
            if (ImGui::Begin("Frustum Culling Info", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE hDescriptor(srvHeap->GetGPUDescriptorHandleForHeapStart());
                hDescriptor.Offset(19, cbvSrvDescriptorSize);
                ImGui::Image((ImTextureID)hDescriptor.ptr, ImVec2((float)304, (float)225));
            } ImGui::End();
        }
    }
    else if (activeSceneID == 10)
    {
        ImVec2 lightPanelSize = ImVec2(350.f, 350.f);
        ImGui::SetNextWindowSize(lightPanelSize);
        if (ImGui::Begin("Point Light Config", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text("Point Light 1");
            if (ImGui::BeginTable("PointLight1", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##PointLightPos1X", &lightPos1Scene10[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##PointLightPos1Y", &lightPos1Scene10[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##PointLightPos1Z", &lightPos1Scene10[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("Distance 1", &light1DistanceScene10, 0.0f, 2.0f);
            ImGui::SliderFloat("Strength 1", &light1StrengthScene10, 0.0f, 1.0f);
            ImGui::ColorPicker3("Color 1", col1Scene10, ImGuiColorEditFlags_NoAlpha);

            ImGui::Text("");
            ImGui::Text("Point Light 2");
            if (ImGui::BeginTable("PointLight2", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##PointLightPos2X", &lightPos2Scene10[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##PointLightPos2Y", &lightPos2Scene10[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##PointLightPos2Z", &lightPos2Scene10[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("Distance 2", &light2DistanceScene10, 0.0f, 2.0f);
            ImGui::SliderFloat("Strength 2", &light2StrengthScene10, 0.0f, 1.0f);
            ImGui::ColorPicker3("Color 2", col2Scene10, ImGuiColorEditFlags_NoAlpha);

            ImGui::Text("");
            ImGui::Text("Point Light 3");
            if (ImGui::BeginTable("PointLight3", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##PointLightPos3X", &lightPos3Scene10[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##PointLightPos3Y", &lightPos3Scene10[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##PointLightPos3Z", &lightPos3Scene10[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("Distance 3", &light3DistanceScene10, 0.0f, 2.0f);
            ImGui::SliderFloat("Strength 3", &light3StrengthScene10, 0.0f, 1.0f);
            ImGui::ColorPicker3("Color 3", col3Scene10, ImGuiColorEditFlags_NoAlpha);

        } ImGui::End();

        ImVec2 spotLightPanelSize = ImVec2(350.f, 350.f);
        ImVec2 spotLightPanelPos = ImVec2(100.f, 200.f);
        ImGui::SetNextWindowSize(spotLightPanelSize);
        ImGui::SetNextWindowPos(spotLightPanelPos);
        if (ImGui::Begin("Spot Light Config", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text("Spot Light 1");
            if (ImGui::BeginTable("SpotLight1", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##SpotLightPos1X", &lightPosSpot1Scene10[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##SpotLightPos1Y", &lightPosSpot1Scene10[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##SpotLightPos1Z", &lightPosSpot1Scene10[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("SpotLightDir1X", &spotLight1DirectionScene10[0], -DirectX::XM_PI, DirectX::XM_PI);
            ImGui::SliderFloat("SpotLightDir1Y", &spotLight1DirectionScene10[1], -DirectX::XM_PI, DirectX::XM_PI);
            ImGui::SliderFloat("SpotLightDir1Z", &spotLight1DirectionScene10[2], -DirectX::XM_PI, DirectX::XM_PI);

            ImGui::SliderFloat("Spot Distance 1", &light1SpotDistanceScene10, 0.0f, 20.0f);
            ImGui::SliderFloat("Spot Strength 1", &light1SpotStrengthScene10, 0.0f, 10.0f);
            ImGui::ColorPicker3("Spot Color 1", colSpot1Scene10, ImGuiColorEditFlags_NoAlpha);

        } ImGui::End();
    }
    else if (activeSceneID == 13)
    {
        if (isActiveOctreeCullingInfoScene13)
        {
            ImVec2 size = ImVec2(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 3);
            ImVec2 pos = ImVec2(WINDOW_WIDTH / 2 - size.x / 2, 0);

            ImGui::SetNextWindowPos(pos);
            ImGui::SetNextWindowSize(size);
            ImGui::SetNextWindowBgAlpha(0.5f);
            if (ImGui::Begin("Octree Culling Info", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE hDescriptor(srvHeap->GetGPUDescriptorHandleForHeapStart());
                hDescriptor.Offset(58, cbvSrvDescriptorSize);
                ImGui::Image((ImTextureID)hDescriptor.ptr, ImVec2((float)304, (float)225));
            } ImGui::End();
        }
    }

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
}
