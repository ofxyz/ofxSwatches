#include "myColor.h"
#include "ofColor.h"
#include <string>

myColor::myColor(glm::vec4 col /*= {0,0,0,0}*/, eSwatchType type, std::string name /*= ""*/)
{
	m_eType = type;
	initColor = col;

	switch (m_eType) {
	case EST_RGB:
		if (name == "") {
			name = "R=" + std::to_string(col.r) + " G=" + std::to_string(col.g) + " B=" + std::to_string(col.b) + " A=" + std::to_string(col.a);
		}
		break;
	case EST_CMYK:
		if (name == "") {
			name = "C=" + std::to_string(col.r) + " M=" + std::to_string(col.g) + " Y=" + std::to_string(col.b) + " K=" + std::to_string(col.a);
		}
		col = CMYK2RGB(col);
		break;
	}

	setName(name);
	set(col.r, col.g, col.b, col.a);
}

myColor::~myColor()
{

}

std::string myColor::getName()
{
	return m_sName;
}

void myColor::setName(std::string name)
{
	m_sName = name;
}

eSwatchType myColor::getType() const
{
	return m_eType;
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