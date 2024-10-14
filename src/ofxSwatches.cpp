#include "ofxSwatches.h"

static void to_json(ofJson& j, const ofColor& color)
{
	j = ofJson{
		{ "r", color.r },
		{ "g", color.g },
		{ "b", color.b },
		{ "a", color.a }
	};
}

static void from_json(const ofJson& j, ofColor& color)
{
	try {
		j.at("r").get_to(color.r);
		j.at("g").get_to(color.g);
		j.at("b").get_to(color.b);
		j.at("a").get_to(color.a);
	}
	catch (const ofJson::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

ofxSwatches::ofxSwatches()
{
	m_sName = "ofxSwatches";
}

ofxSwatches::~ofxSwatches()
{

}

void ofxSwatches::addRGB(glm::vec3 col, std::string name /*= ""*/)
{
	m_vColors.push_back(myColor(glm::vec4(col, 255), EST_RGB, name));
}

void ofxSwatches::addRGB(glm::vec4 col, std::string name /*= ""*/)
{
	m_vColors.push_back(myColor(col, EST_RGB, name));
}

void ofxSwatches::addCMYK(glm::vec4 col, std::string name /*= ""*/)
{
	m_vColors.push_back(myColor(col, EST_CMYK, name));
}

glm::vec4 ofxSwatches::getRGB(std::string& name)
{
	return getColorObject(name);
}

glm::vec4 ofxSwatches::getRGB(int index)
{
	return getColorObject(index);
}

glm::vec4 ofxSwatches::getCMYK(std::string& name)
{
	return getColorObject(name).getCMYK();
}

glm::vec4 ofxSwatches::getCMYK(int index)
{
	return getColorObject(index).getCMYK();
}

ofColor& ofxSwatches::getColor(std::string& name)
{
	myColor col = getColorObject(name);
	return static_cast<ofColor&>(col);
}

ofColor& ofxSwatches::getColor(int index)
{
	myColor col = getColorObject(index);
	return static_cast<ofColor&>(col);
}

myColor& ofxSwatches::getColorObject(std::string& name)
{
	for (size_t i = 0; i < m_vColors.size(); i++)
	{
		if (m_vColors[i].getName() == name) return m_vColors[i];
	}

	throw std::runtime_error("ofxSwatches::Could not find swatch with name: " + name);
}

myColor& ofxSwatches::getColorObject(unsigned int index)
{
	if (index < m_vColors.size())
	{
		return m_vColors[index];
	}
	throw std::runtime_error("ofxSwatches::Index not in range: " + index);
}

ofJson ofxSwatches::getSettings()
{
	ofJson settings;
	settings["libName"] = m_sName;
	for (size_t i = 0; i < m_vColors.size(); i++)
	{
		ofJson j = ofJson {
			{ "name", m_vColors[i].getName() },
			{ "color", (ofColor)m_vColors[i] }
		};
		settings["Swatches"].push_back(j);
	}
	return settings;
}

void ofxSwatches::setSettings(ofJson& settings)
{

}

void ofxSwatches::setLibraryName(std::string& name)
{
	m_sName = name;
}

std::string ofxSwatches::getLibraryName()
{
	return m_sName;
}

void ofxSwatches::saveSettings(std::string path)
{
	if (path.empty())
	{
		path = ofToDataPath(m_sName + ".json").string();
	}

	ofSavePrettyJson(path, getSettings());
}

ofJson ofxSwatches::loadSettings(std::string path)
{
	if (path.empty())
	{
		path = ofToDataPath(m_sName + ".json").string();
	}
	ofJson settings = ofLoadJson(path);
	setSettings(settings);
	return settings;
}

int ofxSwatches::count()
{
	return m_vColors.size();
}
