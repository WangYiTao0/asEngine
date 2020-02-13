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


		animWindow = new asWindow(GUI, "Animation Window");
		animWindow->SetSize(XMFLOAT2(600, 450));
		GUI->AddWidget(animWindow);

		float x = 200;
		float y = 0;
		float step = 35;


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
		loopedCheckBox->SetPos(XMFLOAT2(150, y += step));
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
		playButton->SetPos(XMFLOAT2(200, y));
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
		stopButton->SetPos(XMFLOAT2(310, y));
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
		timerSlider->SetPos(XMFLOAT2(x, y += step * 2));
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