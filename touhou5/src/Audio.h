#pragma once

#include <SFML/Audio.hpp>
#include <filesystem>

namespace th
{
	class Audio
	{
	public:
		void Update();

		void PlayMusic(const std::filesystem::path& fname);
		void PlaySound(const sf::SoundBuffer& buf);
		bool SoundPlaying(const sf::SoundBuffer& buf) const;

		sf::Music music;
		std::vector<sf::Sound> sounds;
	};
}
