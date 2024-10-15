#include "myColor.h"
#include "ofColor.h"
#include <sstream>
#include <iomanip>

myColor::myColor(glm::vec4 col /*= {0,0,0,0}*/, eSwatchType type, std::string name /*= ""*/)
{
	m_eType = type;
	initColor = col;
	useColorValuesForName = (name == "")? true : false;
	m_bSpot = false;

	if (m_eType == EST_CMYK) {
		col = CMYK2RGB(col);
	}

	set(col.r, col.g, col.b, col.a);
	setName(name);
}

myColor::~myColor()
{

}

std::string& myColor::getName()
{
	return m_sName;
}

std::string myColor::getNameStr() const
{
	return m_sName;
}

void myColor::setName(std::string name /*=""*/)
{
	if (name == "") {
		if (useColorValuesForName) {
			// Update
			m_sName = getValueName();
		}
		return;
	}
	m_sName = name;
}

std::string myColor::getValueName()
{
	std::string name = "";
	switch (m_eType) {
	case EST_RGB:
		name = "R=" + stringDec((float)this->r, 2) + " G=" + stringDec((float)this->g, 2) + " B=" + stringDec((float)this->b, 2) + " A=" + stringDec((float)this->a, 2);
		break;
	case EST_CMYK:
		name = "C=" + stringDec((float)this->r, 2) + " M=" + stringDec((float)this->g, 2) + " Y=" + stringDec((float)this->b, 2) + " K=" + stringDec((float)this->a, 2);
		break;
	}
	return name;
}

eSwatchType myColor::getType() const
{
	return m_eType;
}

void myColor::setType(eSwatchType type)
{
	m_eType = type;
}

glm::vec4 myColor::getCMYK()
{
	// Convert RGB to CMYK
	float c = 1 - r;
	float m = 1 - g;
	float y = 1 - b;
	float k = std::min(c, std::min(m, y));
	if (k == 1) {
		// TODO: Create rich black setting? We need some control over this.
		return glm::vec4(0, 0, 0, 1);
	}
	else {
		return glm::vec4((c - k) / (1 - k), (m - k) / (1 - k), (y - k) / (1 - k), k);
	}
}

glm::vec4 myColor::CMYK2RGB(glm::vec4 colCMYK)
{
	float k = colCMYK.a/100;

	float r = (255 * (1 - (colCMYK.r/100)) * (1 - k));
	float g = (255 * (1 - (colCMYK.g)/100) * (1 - k));
	float b = (255 * (1 - (colCMYK.b)/100) * (1 - k));

	return glm::vec4(r, g, b, 255);
}

bool myColor::isSpot() const
{
	return m_bSpot;
}

void myColor::setSpot(bool spot)
{
	m_bSpot = spot;
}

std::string myColor::stringDec(float val, unsigned int precision)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << val;
	return stream.str();
}
