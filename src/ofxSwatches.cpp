#include "ofxSwatches.h"

static void to_json(ofJson& j, const myColor& color)
{
	j = ofJson{
		{ "name", color.getNameStr() },
		{ "type", color.getType() },
		{ "spot", color.isSpot() },
		{ "r", color.r },
		{ "g", color.g },
		{ "b", color.b },
		{ "a", color.a }
	};
}

static void from_json(const ofJson& j, myColor& color)
{
	try {
		color.setType(j.at("type").get<eSwatchType>());
		color.setName(j.at("name").get<std::string>());
		color.setSpot(j.at("spot").get<bool>());
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
	m_sName = "Untitled";
	isOpen = true;
}

ofxSwatches::~ofxSwatches()
{

}

void ofxSwatches::addRGB(glm::vec3 col, std::string name /*= ""*/)
{
	/*
	name = getColorName(glm::vec4(col, 255), EST_RGB, name);
	
	if (colorExists(name)) {
		ofLog(OF_LOG_WARNING) << "ofxSwatches::Swatch with name: " << name << " already exists.";
		return;
	}
	*/

	m_vColors.push_back(myColor(glm::vec4(col, 255), EST_RGB, name));
}

void ofxSwatches::addRGB(glm::vec4 col, std::string name /*= ""*/)
{
	/*
	name = getColorName(col, EST_RGB, name);

	if (colorExists(name)) {
		ofLog(OF_LOG_WARNING) << "ofxSwatches::Swatch with name: " << name << " already exists.";
		return;
	}
	*/
	m_vColors.push_back(myColor(col, EST_RGB, name));
}

void ofxSwatches::addCMYK(glm::vec4 col, std::string name /*= ""*/)
{
	/*
	name = getColorName(col, EST_RGB, name);

	if (colorExists(name)) {
		ofLog(OF_LOG_WARNING) << "ofxSwatches::Swatch with name: " << name << " already exists.";
		return;
	}
	*/
	m_vColors.push_back(myColor(col, EST_CMYK, name));
}

void ofxSwatches::removeSwatch(std::string name)
{
	for (size_t i = 0; i < m_vColors.size(); i++)
	{
		if (m_vColors[i].getName() == name)
		{
			m_vColors.erase(m_vColors.begin() + i);
			return;
		}
	}
	ofLog(OF_LOG_WARNING) << "ofxSwatches::Could not delete swatch with name: " << name;
}

void ofxSwatches::removeSwatch(int index)
{
	if (index < 0) index = m_vColors.size() - index;
	if (index < m_vColors.size()) {
		m_vColors.erase(m_vColors.begin() + index);
		return;
	}
	ofLog(OF_LOG_WARNING) << "ofxSwatches::Could not delete swatch with index: " << index;
}

glm::vec4 ofxSwatches::getRGB(std::string name)
{
	return getColorObject(name);
}

glm::vec4 ofxSwatches::getRGB(int index)
{
	return getColorObject(index);
}

glm::vec4 ofxSwatches::getCMYK(std::string name)
{
	return getColorObject(name).getCMYK();
}

glm::vec4 ofxSwatches::getCMYK(int index)
{
	return getColorObject(index).getCMYK();
}

ofColor& ofxSwatches::getColor(std::string name)
{
	myColor col = getColorObject(name);
	return static_cast<ofColor&>(col);
	//return glm::vec4(col.r,col.g,col.b,col.a);
}

ofColor& ofxSwatches::getColor(int index)
{
	myColor col = getColorObject(index);
	return static_cast<ofColor&>(col);
}

myColor& ofxSwatches::getColorObject(std::string name)
{
	for (size_t i = 0; i < m_vColors.size(); i++)
	{
		if (m_vColors[i].getName() == name) return m_vColors[i];
	}

	ofLog(OF_LOG_FATAL_ERROR) << "ofxSwatches: Could not find color with name " << name << " in swatches.";
	// Return white not found colour
	myColor r = myColor(glm::vec4(255, 255, 255, 255), EST_RGB, "Not Found");
	return r;
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
	for (myColor mc : m_vColors) {
		settings["Swatches"].push_back(mc);
	}
	return settings;
}

void ofxSwatches::setSettings(ofJson& settings)
{

}

bool ofxSwatches::colorExists(std::string name)
{
	for (myColor swatch : m_vColors) {
		if (swatch.getName() == name) return true;
	}
	return false;
}

void ofxSwatches::setLibraryName(std::string name)
{
	m_sName = name;
}

std::string& ofxSwatches::getLibraryName()
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

std::vector<myColor>& ofxSwatches::getSwatches()
{
	return m_vColors;
}

std::string ofxSwatches::getColorName(glm::vec4 col, eSwatchType type, std::string& name)
{
	if (name == "") {
		switch (type) {
		case EST_RGB:
			name = "R=" + myColor::stringDec((float)col.r, 2) + " G=" + myColor::stringDec((float)col.g, 2) + " B=" + myColor::stringDec((float)col.b, 2) + " A=" + myColor::stringDec((float)col.a, 2);
			break;
		case EST_CMYK:
			name = "C=" + myColor::stringDec((float)col.r, 2) + " M=" + myColor::stringDec((float)col.g, 2) + " Y=" + myColor::stringDec((float)col.b, 2) + " K=" + myColor::stringDec((float)col.a, 2);
			break;
		}
		return name;
	}
	return name;
}

int ofxSwatches::count()
{
	return m_vColors.size();
}
