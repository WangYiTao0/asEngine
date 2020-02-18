#include "AnimationWindow.h"

#include <sstream>
namespace as
{
	using namespace asECS;
	using namespace asScene;

	AnimationWindow::AnimationWindow(asGUI* gui) :GUI(gui)
	{
		assert(GUI && "Invalid GUI!");

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

		XMFLOAT2 option_size = XMFLOAT2(100, 20);
		XMFLOAT2 slider_size = XMFLOAT2(200, 20);
		animWindow = new asWindow(GUI, "Animation Window");
		animWindow->SetSize(XMFLOAT2(500, 250));

		float windowX = screenW / 3.0f - animWindow->GetScale().x / 2.0f;
		float windowY = 50;

		animWindow->SetPos(XMFLOAT2(windowX, windowY));
		GUI->AddWidget(animWindow);

		float x = 150 + windowX;;
		float y = 0 + windowY;
		float step = 22;


		animationsComboBox = new asComboBox("Animation: ");
		animationsComboBox->SetSize(XMFLOAT2(300, 20));
		animationsComboBox->SetPos(XMFLOAT2(x, y += step));
		animationsComboBox->SetEnabled(false);
		animationsComboBox->OnSelect([&](asEventArgs args) {
			entity = asScene::GetScene().animations.GetEntity(args.iValue);
			});
		animationsComboBox->SetTooltip("Choose an animation clip...");
		animWindow->AddWidget(animationsComboBox);

		loopedCheckBox = new asCheckBox("Looped: ");
		loopedCheckBox->SetTooltip("Toggle animation looping behaviour.");
		loopedCheckBox->SetPos(XMFLOAT2(x-50, y += step * 2));
		loopedCheckBox->OnClick([&](asEventArgs args) {
			AnimationComponent* animation = asScene::GetScene().animations.GetComponent(entity);
			if (animation != nullptr)
			{
				animation->SetLooped(args.bValue);
			}
			});
		animWindow->AddWidget(loopedCheckBox);

		playButton = new asButton("Play");
		playButton->SetTooltip("Play/Pause animation.");
		playButton->SetSize(XMFLOAT2(80, 20));
		playButton->SetPos(XMFLOAT2(x+=50, y));

		playButton->OnClick([&](asEventArgs args) {
			AnimationComponent* animation = asScene::GetScene().animations.GetComponent(entity);
			if (animation != nullptr)
			{
				if (animation->IsPlaying())
				{
					animation->Pause();
				}
				else
				{
					animation->Play();
				}
			}
			});
		animWindow->AddWidget(playButton);

		stopButton = new asButton("Stop");
		stopButton->SetTooltip("Stop animation.");
		stopButton->SetPos(XMFLOAT2(x+=100, y));
		stopButton->SetSize(XMFLOAT2(80, 20));
		stopButton->OnClick([&](asEventArgs args) {
			AnimationComponent* animation = asScene::GetScene().animations.GetComponent(entity);
			if (animation != nullptr)
			{
				animation->Stop();
			}
			});
		animWindow->AddWidget(stopButton);

		timerSlider = new asSlider(0, 1, 0, 100000, "Timer: ");
		timerSlider->SetSize(XMFLOAT2(250, 30));
		timerSlider->SetPos(XMFLOAT2(x - 200, y += step * 2));
		timerSlider->OnSlide([&](asEventArgs args) {
			AnimationComponent* animation = asScene::GetScene().animations.GetComponent(entity);
			if (animation != nullptr)
			{
				animation->timer = args.fValue;
			}
			});
		timerSlider->SetEnabled(false);
		timerSlider->SetTooltip("Set the animation timer by hand.");
		animWindow->AddWidget(timerSlider);



		animWindow->Translate(XMFLOAT3(100, 50, 0));
		animWindow->SetVisible(false);

	}


	AnimationWindow::~AnimationWindow()
	{
		animWindow->RemoveWidgets(true);

		GUI->RemoveWidget(animWindow);
		SAFE_DELETE(animWindow);
	}

	void AnimationWindow::Update()
	{
		animationsComboBox->ClearItems();

		Scene& scene = asScene::GetScene();

		if (!scene.animations.Contains(entity))
		{
			entity = INVALID_ENTITY;
		}

		if (scene.animations.GetCount() == 0)
		{
			animWindow->SetEnabled(false);
			return;
		}
		else
		{
			animWindow->SetEnabled(true);
		}

		for (size_t i = 0; i < scene.animations.GetCount(); ++i)
		{
			Entity e = scene.animations.GetEntity(i);
			NameComponent& name = *scene.names.GetComponent(e);

			std::stringstream ss("");
			ss << name.name << " (" << e << ")";
			animationsComboBox->AddItem(ss.str());

			if (e == entity)
			{
				animationsComboBox->SetSelected((int)i);
			}
		}

		if (entity == INVALID_ENTITY && scene.animations.GetCount() > 0)
		{
			entity = scene.animations.GetEntity(0);
			animationsComboBox->SetSelected(0);
		}

		int selected = animationsComboBox->GetSelected();
		if (selected >= 0 && selected < (int)scene.animations.GetCount())
		{
			AnimationComponent& animation = scene.animations[selected];

			if (animation.IsPlaying())
			{
				playButton->SetText("Pause");
			}
			else
			{
				playButton->SetText("Play");
			}

			loopedCheckBox->SetCheck(animation.IsLooped());

			timerSlider->SetRange(0, animation.GetLength());
			timerSlider->SetValue(animation.timer);
		}
	}
}