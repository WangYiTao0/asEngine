#include <asEngine.h>
#include "EnvProbeWindow.h"

namespace as
{
	using namespace asECS;
	using namespace asScene;

	EnvProbeWindow::EnvProbeWindow(asGUI* gui) : GUI(gui)
	{
		assert(GUI && "Invalid GUI!");

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

		envProbeWindow = new asWindow(GUI, "Environment Probe Window");
		envProbeWindow->SetSize(XMFLOAT2(600, 400));
		GUI->AddWidget(envProbeWindow);

		float x = 250, y = 0, step = 45;

		realTimeCheckBox = new asCheckBox("RealTime: ");
		realTimeCheckBox->SetPos(XMFLOAT2(x, y += step));
		realTimeCheckBox->SetEnabled(false);
		realTimeCheckBox->OnClick([&](asEventArgs args) {
			EnvironmentProbeComponent* probe = asScene::GetScene().probes.GetComponent(entity);
			if (probe != nullptr)
			{
				probe->SetRealTime(args.bValue);
				probe->SetDirty();
			}
			});
		envProbeWindow->AddWidget(realTimeCheckBox);

		generateButton = new asButton("Put");
		generateButton->SetPos(XMFLOAT2(x, y += step));
		generateButton->OnClick([](asEventArgs args) {
			XMFLOAT3 pos;
			XMStoreFloat3(&pos, XMVectorAdd(asRenderer::GetCamera().GetEye(), asRenderer::GetCamera().GetAt() * 4));
			asScene::GetScene().Entity_CreateEnvironmentProbe("editorProbe", pos);
			});
		envProbeWindow->AddWidget(generateButton);

		refreshButton = new asButton("Refresh");
		refreshButton->SetPos(XMFLOAT2(x, y += step));
		refreshButton->SetEnabled(false);
		refreshButton->OnClick([&](asEventArgs args) {
			EnvironmentProbeComponent* probe = asScene::GetScene().probes.GetComponent(entity);
			if (probe != nullptr)
			{
				probe->SetDirty();
			}
			});
		envProbeWindow->AddWidget(refreshButton);

		refreshAllButton = new asButton("Refresh All");
		refreshAllButton->SetPos(XMFLOAT2(x, y += step));
		refreshAllButton->SetEnabled(true);
		refreshAllButton->OnClick([&](asEventArgs args) {
			Scene& scene = asScene::GetScene();
			for (size_t i = 0; i < scene.probes.GetCount(); ++i)
			{
				EnvironmentProbeComponent& probe = scene.probes[i];
				probe.SetDirty();
			}
			});
		envProbeWindow->AddWidget(refreshAllButton);




		envProbeWindow->Translate(XMFLOAT3(30, 30, 0));
		envProbeWindow->SetVisible(false);

		SetEntity(INVALID_ENTITY);
	}


	EnvProbeWindow::~EnvProbeWindow()
	{
		envProbeWindow->RemoveWidgets(true);
		GUI->RemoveWidget(envProbeWindow);
		SAFE_DELETE(envProbeWindow);
	}

	void EnvProbeWindow::SetEntity(Entity entity)
	{
		this->entity = entity;

		const EnvironmentProbeComponent* probe = asScene::GetScene().probes.GetComponent(entity);

		if (probe == nullptr)
		{
			realTimeCheckBox->SetEnabled(false);
			refreshButton->SetEnabled(false);
		}
		else
		{
			realTimeCheckBox->SetCheck(probe->IsRealTime());
			realTimeCheckBox->SetEnabled(true);
			refreshButton->SetEnabled(true);
		}
	}
}
