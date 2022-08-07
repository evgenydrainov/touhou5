#include "Audio.h"

namespace th
{
	void Audio::Update()
	{
		for (auto s = sounds.begin(); s != sounds.end();) {
			if (s->getStatus() == sf::Sound::Playing) {
				++s;
			} else {
				s = sounds.erase(s);
			}
		}
	}

	void Audio::PlayMusic(const std::filesystem::path& fname)
	{
		music.openFromFile(fname.string());
		music.play();
	}

	void Audio::PlaySound(const sf::SoundBuffer& buf)
	{
		for (auto s = sounds.begin(); s != sounds.end();) {
			if (s->getBuffer() == &buf) {
				s = sounds.erase(s);
			} else {
				++s;
			}
		}
		sf::Sound& s = sounds.emplace_back();
		s.setBuffer(buf);
		s.play();
	}

	bool Audio::SoundPlaying(const sf::SoundBuffer& buf) const
	{
		for (const sf::Sound& s : sounds) {
			if (s.getBuffer() == &buf) {
				return true;
			}
		}
		return false;
	}
}
