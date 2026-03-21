#define NUM_OF_ATTENUATIONS 12

#include <vector>

struct Attenuation {
    float minDist;
    float maxDist;
    float constant;
    float linear;
    float quadratic;
};

extern std::vector<Attenuation> attenuationData;

float CalculateAttenuation(float distance);