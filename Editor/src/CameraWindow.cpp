#include <asEngine.h>
#include "CameraWindow.h"
namespace as
{

	using namespace asECS;
	using namespace asScene;

	void CameraWindow::ResetCam()
	{
		Scene& scene = asScene::GetScene();

		camera_transform.ClearTransform();
		camera_transform.Translate(XMFLOAT3(0, 2, -10));
		camera_transform.UpdateTransform();
		asRenderer::GetCamera().TransformCamera(camera_transform);

		camera_target.ClearTransform();
		camera_target.UpdateTransform();
	}

	CameraWindow::CameraWindow(asGUI* gui) :GUI(gui)
	{
		assert(GUI && "Invalid GUI!");

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

		camera_transform.MatrixTransform(asRenderer::GetCamera().GetInvView());
		camera_transform.UpdateTransform();

		cameraWindow = new asWindow(GUI, "Camera Window");
		cameraWindow->SetSize(XMFLOAT2(600, 420));
		GUI->AddWidget(cameraWindow);

		float x = 200;
		float y = 0;
		float inc = 35;

		farPlaneSlider = new asSlider(1, 5000, 1000, 100000, "Far Plane: ");
		farPlaneSlider->SetSize(XMFLOAT2(100, 30));
		farPlaneSlider->SetPos(XMFLOAT2(x, y += inc));
		farPlaneSlider->SetValue(asRenderer::GetCamera().zFarP);
		farPlaneSlider->OnSlide([&](asEventArgs args) {
			Scene& scene = asScene::GetScene();
			CameraComponent& camera = asRenderer::GetCamera();
			camera.zFarP = args.fValue;
			camera.UpdateCamera();
			});
		cameraWindow->AddWidget(farPlaneSlider);

		nearPlaneSlider = new asSlider(0.01f, 10, 0.1f, 10000, "Near Plane: ");
		nearPlaneSlider->SetSize(XMFLOAT2(100, 30));
		nearPlaneSlider->SetPos(XMFLOAT2(x, y += inc));
		nearPlaneSlider->SetValue(asRenderer::GetCamera().zNearP);
		nearPlaneSlider->OnSlide([&](asEventArgs args) {
			Scene& scene = asScene::GetScene();
			CameraComponent& camera = asRenderer::GetCamera();
			camera.zNearP = args.fValue;
			camera.UpdateCamera();
			});
		cameraWindow->AddWidget(nearPlaneSlider);

		fovSlider = new asSlider(1, 179, 60, 10000, "FOV: ");
		fovSlider->SetSize(XMFLOAT2(100, 30));
		fovSlider->SetPos(XMFLOAT2(x, y += inc));
		fovSlider->OnSlide([&](asEventArgs args) {
			Scene& scene = asScene::GetScene();
			CameraComponent& camera = asRenderer::GetCamera();
			camera.fov = args.fValue / 180.f * XM_PI;
			camera.UpdateCamera();
			});
		cameraWindow->AddWidget(fovSlider);

		movespeedSlider = new asSlider(1, 100, 10, 10000, "Movement Speed: ");
		movespeedSlider->SetSize(XMFLOAT2(100, 30));
		movespeedSlider->SetPos(XMFLOAT2(x, y += inc));
		cameraWindow->AddWidget(movespeedSlider);

		rotationspeedSlider = new asSlider(0.1f, 2, 1, 10000, "Rotation Speed: ");
		rotationspeedSlider->SetSize(XMFLOAT2(100, 30));
		rotationspeedSlider->SetPos(XMFLOAT2(x, y += inc));
		cameraWindow->AddWidget(rotationspeedSlider);

		resetButton = new asButton("Reset Camera");
		resetButton->SetSize(XMFLOAT2(140, 30));
		resetButton->SetPos(XMFLOAT2(x, y += inc));
		resetButton->OnClick([&](asEventArgs args) {
			ResetCam();
			});
		cameraWindow->AddWidget(resetButton);

		fpsCheckBox = new asCheckBox("FPS Camera: ");
		fpsCheckBox->SetPos(XMFLOAT2(x, y += inc));
		fpsCheckBox->SetCheck(true);
		cameraWindow->AddWidget(fpsCheckBox);



		proxyButton = new asButton("Place Proxy");
		proxyButton->SetTooltip("Copy the current camera and place a proxy of it in the world.");
		proxyButton->SetSize(XMFLOAT2(140, 30));
		proxyButton->SetPos(XMFLOAT2(x, y += inc * 2));
		proxyButton->OnClick([&](asEventArgs args) {

			const CameraComponent& camera = asRenderer::GetCamera();

			Scene& scene = asScene::GetScene();

			Entity entity = scene.Entity_CreateCamera("cam", camera.width, camera.height, camera.zNearP, camera.zFarP, camera.fov);

			TransformComponent& transform = *scene.transforms.GetComponent(entity);
			transform.MatrixTransform(camera.InvView);
			});
		cameraWindow->AddWidget(proxyButton);

		proxyNameField = new asTextInputField("Proxy Name: ");
		proxyNameField->SetSize(XMFLOAT2(140, 30));
		proxyNameField->SetPos(XMFLOAT2(x + 200, y));
		proxyNameField->OnInputAccepted([&](asEventArgs args) {
			Scene& scene = asScene::GetScene();
			NameComponent* camera = scene.names.GetComponent(proxy);
			if (camera != nullptr)
			{
				*camera = args.sValue;
			}
			});
		cameraWindow->AddWidget(proxyNameField);


		followCheckBox = new asCheckBox("Follow Proxy: ");
		followCheckBox->SetPos(XMFLOAT2(x, y += inc));
		followCheckBox->SetCheck(false);
		cameraWindow->AddWidget(followCheckBox);

		followSlider = new asSlider(0.0f, 0.999f, 0.0f, 1000.0f, "Follow Proxy Delay: ");
		followSlider->SetSize(XMFLOAT2(100, 30));
		followSlider->SetPos(XMFLOAT2(x, y += inc));
		cameraWindow->AddWidget(followSlider);


		SetEntity(INVALID_ENTITY);


		cameraWindow->Translate(XMFLOAT3(screenW - 720, 500, 0));
		cameraWindow->SetVisible(false);
	}


	CameraWindow::~CameraWindow()
	{
		cameraWindow->RemoveWidgets(true);
		GUI->RemoveWidget(cameraWindow);
		SAFE_DELETE(cameraWindow);
	}

	void CameraWindow::SetEntity(Entity entity)
	{
		proxy = entity;

		Scene& scene = asScene::GetScene();

		if (scene.cameras.GetComponent(entity) != nullptr)
		{
			followCheckBox->SetEnabled(true);
			followSlider->SetEnabled(true);
			NameComponent* camera = scene.names.GetComponent(entity);
			if (camera != nullptr)
			{
				proxyNameField->SetValue(camera->name);
			}
		}
		else
		{
			followCheckBox->SetCheck(false);
			followCheckBox->SetEnabled(false);
			followSlider->SetEnabled(false);
			proxyNameField->SetValue("Proxy Name: ");
		}
	}
}