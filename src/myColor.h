#pragma once

#include "ofColor.h"
#include "ofJson.h"

enum eSwatchType {
	EST_RGB = 0,
	EST_CMYK,
	EST_Count
};

class myColor: public ofColor {
	public:
		myColor(glm::vec4 col = {0,0,0,0}, eSwatchType type = EST_RGB, std::string name = "");
		~myColor();

		std::string& getName();
		std::string getNameStr() const;
		void setName(std::string name = "");
		std::string getValueName();
		eSwatchType getType() const;
		void setType(eSwatchType type);
		glm::vec4 getCMYK();
		glm::vec4 CMYK2RGB(glm::vec4 colCMYK);
		bool isSpot() const;
		void setSpot(bool spot);
		static std::string stringDec(float val, unsigned int precision);

		operator glm::vec4() const {
			// We could automatically convert to CMYK here bases on the type
			// But I think it's better to have the user do this explicitly
			// Most calculations are done in RGB
			return glm::vec4(r, g, b, a);
		}

	private:
		bool m_bSpot;
		bool useColorValuesForName;
		glm::vec4 initColor;
		std::string m_sName;
		eSwatchType m_eType;
};
