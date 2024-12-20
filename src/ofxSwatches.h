#pragma once

#include "ofJson.h"
#include <string>
#include "myColor.h"

class ofxSwatches
{
	public:
		ofxSwatches(std::string name = "");
		~ofxSwatches();

		int count();

		void addRGB(glm::vec3 col, std::string name = "");
		void addRGB(glm::vec4 col, std::string name = "");
		void addCMYK(glm::vec4 col, std::string name = "");

		void removeSwatch(std::string name);
		void removeSwatch(int index);

		glm::vec4 getRGB(std::string name);
		glm::vec4 getCMYK(std::string name);
		ofColor& getColor(std::string name);

		glm::vec4 getRGB(int index);
		glm::vec4 getCMYK(int index);
		ofColor& getColor(int index);

		myColor& getColorObject(std::string name);
		myColor& getColorObject(unsigned int index);

		ofJson getSettings();
		void setSettings(ofJson& settings);

		bool colorExists(std::string name);

		void setLibraryName(std::string name);
		std::string& getLibraryName();

		// Saves in ofDataPath if no path is defined overwriting whatever is there.
		void saveSettings(std::string path = "");
		ofJson loadSettings(std::string path = "");

		std::vector<myColor>& getSwatches();
		bool isOpen;
		
		std::string getColorName(glm::vec4 col, eSwatchType type, std::string& name);

	private:
		std::string m_sName;
		std::vector<myColor> m_vColors;
};
