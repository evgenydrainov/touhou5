#include "ErrorScene.h"

namespace th
{
	void ErrorScene::Init()
	{
		text.setFont(game.font);
		text.setCharacterSize(14);
		text.setString(what);
	}

	void ErrorScene::Update(float delta)
	{
	}

	void ErrorScene::Render(sf::RenderTarget& target, float delta)
	{
		target.draw(text);
	}
}
