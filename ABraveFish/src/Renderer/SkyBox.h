#pragma once

#include "Shader.h"

namespace ABraveFish {
	class SkyBox {

		public:
            void loadCubeMap(const char* filename);
            
            CubeMap _enviromentMap;
        };
} // namespace ABraveFish