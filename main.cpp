#include <iostream>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Libs/stb_image.h"
#include "Libs/stb_image_write.h"

#define byte unsigned char
#define PI 3.145
std::string OUTPUT_PATH = "../Output/";

struct Ray {
    glm::dvec3 origin;
    glm::dvec3 direction;

    Ray(glm::dvec3 origin, glm::dvec3 direction) : origin(origin), direction(direction) {

    }
};

struct Sphere {
    glm::dvec3 center;
    double radius;
};

bool raySphereIntersection(Ray ray, Sphere s, glm::dvec3 &point) {
    glm::dvec3 vec1 = (ray.origin - s.center);
    double a = glm::dot(ray.direction, ray.direction);
    double b = glm::dot(ray.direction, vec1) * 2.0;
    double c = glm::dot(vec1, vec1) - (s.radius * s.radius);

    double delta = (b * b) - (4 * a * c);
    if (delta < 0) {
        point = glm::dvec3(NAN, NAN, NAN);
        return false;
    }
    delta = sqrt(delta);
    double div = (2 * a);
    double r1 = (-b + delta) / div;
    double r2 = (-b - delta) / div;
    if (r1 < 0) r1 = r2;
    if (r2 < 0) r2 = r1;
    if (r2 < 0 && r1 < 0) {
        point = glm::dvec3(NAN, NAN, NAN);
        return false;
    }
    if (r1 <= r2) {
        point = ray.origin + r1 * ray.direction;
        return true;
    }
    point = ray.origin + r2 * ray.direction;
    return true;
}

struct Image {
    int width = 1920;
    int height = 1080;
    int channels = 3;
    byte *data = new byte[width * height * channels];

    void pixel(int x, int y, glm::dvec3 color) {
        int index = (y * width + x) * channels;
        this->data[index + 0] = (byte) color.x;
        this->data[index + 1] = (byte) color.y;
        this->data[index + 2] = (byte) color.z;

    }

    void write(std::string name) {
        stbi_write_png(name.c_str(), width, height, channels, data, width * channels);
    }
};

int main() {
    Image output;
    double fieldOfView = 90;
    double focalLength = 1;
    double aspectRatio = output.width /  (float)output.height;
    Sphere s = {glm::dvec3(0,0,-4),2};
    for (int y = 0; y < output.height; ++y) {
        for (int x = 0; x < output.width; ++x) {
            // Convert the raster coordinates to [-1,1] range considering the aspect ratio and invert the Y coordinate
            glm::dvec2 screenCoord = glm::vec2((((x + 0.5) / output.width) * 2) - 1, (((output.height - y + 0.5) / output.height) * 2) - 1);
            // Consider the camera field of view and focal length/distance
            glm::dvec2 cameraCoord = screenCoord * tan(focalLength * (fieldOfView / 2.0) * (PI / 180));
            Ray r = Ray(glm::dvec3(0,0,0),glm::normalize(glm::dvec3(cameraCoord.x * aspectRatio, cameraCoord.y, -1)));
            glm::dvec3 intersection;
            if(raySphereIntersection(r,s,intersection)){
                output.pixel(x, y, glm::dvec3(255, 0, 0));
            }else{
                output.pixel(x, y, glm::dvec3(0, 0, 255));
            }
        }
    }
    output.write(OUTPUT_PATH + "teste.png");
    return 0;
}
