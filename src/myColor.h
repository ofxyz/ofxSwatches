#pragma once

#include "ofColor.h"

enum eSwatchType {
	EST_RGB = 0,
	EST_CMYK,
	EST_SPOT,
	EST_Count
};

class myColor: public ofColor {
	public:
		myColor(glm::vec4 col = {0,0,0,0}, eSwatchType type = EST_RGB, std::string name = "");
		~myColor();
		std::string getName();
		void setName(std::string name);
		eSwatchType getType() const;
		glm::vec4 getCMYK();
		glm::vec4 CMYK2RGB(glm::vec4 colCMYK);
		// Conversion operator to glm::vec4
		operator glm::vec4() const {
			// Standard RGB
			return glm::vec4(r, g, b, a);
		}
	private:
		glm::vec4 initColor;
		std::string m_sName;
		eSwatchType m_eType;
};
