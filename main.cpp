#include <iostream>
#include <glm/glm.hpp>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Libs/stb_image.h"
#include "Libs/stb_image_write.h"

#define byte unsigned char
#define PI 3.145
std::string OUTPUT_PATH = "../Output/";

class PointLight {
public:
    glm::dvec3 position;
    glm::dvec3 color = {1, 1, 1};

    PointLight(const glm::dvec3 &position, const glm::dvec3 &color) : position(position), color(color) {}
};


class Ray {
public:
    glm::dvec3 origin;
    glm::dvec3 direction;

    Ray(glm::dvec3 origin, glm::dvec3 direction) : origin(origin), direction(direction) {}
};

class Material {
public:
    glm::dvec3 albedo;
    double reflection;
    double refraction;
    double diffuse;
    double refractiveIndex;

    Material(const glm::dvec3 &albedo, double diffuse, double reflection, double refraction, double refractiveIndex) : albedo(albedo), diffuse(diffuse), reflection(reflection),
                                                                                                                       refraction(refraction), refractiveIndex(refractiveIndex) {}

    Material() {}
};

class HitInfo {
public:
    double t;
    //This variable indicates if the ray intersected with the inside or outside of an object
    bool insideObject;
    glm::dvec3 normal;
    glm::dvec3 point;
    Material material;
};

class Hittable {
public:
    virtual bool hit(const Ray &r, HitInfo &info) = 0;
};

class Sphere : public Hittable {
public:
    glm::dvec3 center;
    double radius;
    Material material;

    bool hit(const Ray &r, HitInfo &info) override {
        glm::dvec3 vec1 = (r.origin - center);
        double a = glm::dot(r.direction, r.direction);
        double b = glm::dot(r.direction, vec1) * 2.0;
        double c = glm::dot(vec1, vec1) - (radius * radius);

        double delta = (b * b) - (4 * a * c);
        if (delta < 0) return false;
        delta = sqrt(delta);
        double div = (2 * a);
        double r1 = (-b + delta) / div;
        double r2 = (-b - delta) / div;
        if (r1 < 0) r1 = r2;
        if (r2 < 0) r2 = r1;
        if (r2 < 0 && r1 < 0) return false;
        if (r1 <= r2) {
            info.t = r1;
        } else {
            info.t = r2;
        }
        info.point = r.origin + r.direction * info.t;
        info.normal = glm::normalize(info.point - center);
        info.insideObject = false;
        if (glm::dot(info.normal, r.direction) > 0) {
            info.insideObject = true;
            info.normal = -info.normal;
        }
        info.material = material;
        return true;
    }

    Sphere(glm::dvec3 center, double radius, Material material) : center(center), radius(radius), material(material) {}
};

struct Image {
public:
    int width;
    int height;
    int channels = 3;
    byte *data = new byte[width * height * channels];

    Image(int width, int height) : width(width), height(height) {

    }

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

bool getClosestHit(Ray r, const std::vector<Hittable *> &sceneObjects, HitInfo &hitInfo) {
    HitInfo curHit{10000};
    bool hit = false;
    for (int i = 0; i < sceneObjects.size(); ++i) {
        Hittable *object = sceneObjects[i];
        if (object->hit(r, curHit)) {
            hit = true;
            if (curHit.t < hitInfo.t) {
                hitInfo = curHit;
            }
        }
    }
    return hit;
}

glm::dvec3 BACKGROUND_COLOR = glm::dvec3(0.2, 0.7, 0.8);
double AMBIENT_INTENSITY = 0.05;

glm::dvec3 reflectVector(glm::dvec3 i, glm::dvec3 n) {
    return i - 2 * glm::dot(i, n) * n;
}

// https://en.wikipedia.org/wiki/Snell%27s_law
glm::dvec3 refractVector(glm::dvec3 i, glm::dvec3 n, double n1, double n2) {
    double nr = n1 / n2;
    double cosI = -glm::dot(i,n);
    double totalRefTerm = 1 - (nr * nr) * (1 - (cosI * cosI));
    if (totalRefTerm < 0)
        return glm::dvec3(0,0,0);
    totalRefTerm = sqrt(totalRefTerm);
    return nr * i + n * (nr * cosI - totalRefTerm);
}


glm::dvec3 traceRay(Ray r, const std::vector<Hittable *> &sceneObjects, const std::vector<PointLight> &lights, int curDepth, int depth = 5) {
    if (curDepth > depth) return BACKGROUND_COLOR;
    curDepth++;
    HitInfo info{100000};
    glm::dvec3 diffuseIntensity = glm::dvec3(0, 0, 0);
    // Diffuse calculation
    if (getClosestHit(r, sceneObjects, info)) {
        for (int i = 0; i < lights.size(); ++i) {
            PointLight light = lights[i];
            glm::dvec3 lightVector = glm::normalize(light.position - info.point);
            glm::dvec3 rayOrigin = info.point + info.normal * 0.0001;
            Ray shadowRay = Ray(rayOrigin, lightVector);
            HitInfo shadowHit;
            bool hit = getClosestHit(shadowRay, sceneObjects, shadowHit);
            if (!hit) {
                diffuseIntensity += light.color * glm::max(glm::dot(info.normal, lightVector), 0.0);
            }
        }
        Ray reflectRay = Ray(info.point + info.normal * 0.0001, reflectVector(r.direction, info.normal));
        glm::dvec3 specIntensity = traceRay(reflectRay, sceneObjects, lights, curDepth);
        double n1 = info.insideObject ? info.material.refractiveIndex : 1;
        double n2 = info.insideObject ? 1 : info.material.refractiveIndex;
        glm::dvec3 rayOrigin = info.point - info.normal * 0.0001;
        Ray refractRay = Ray(rayOrigin, refractVector(r.direction, info.normal, n1, n2));
        glm::dvec3 refractIntensity = traceRay(refractRay, sceneObjects, lights, curDepth);
        return info.material.albedo * AMBIENT_INTENSITY + info.material.albedo * diffuseIntensity * info.material.diffuse + info.material.reflection * specIntensity + info.material.refraction * refractIntensity;
    }

    return BACKGROUND_COLOR;
}


int main() {
    Image output(1920, 1080);
    double fieldOfView = 90;
    double focalLength = 1;
    double aspectRatio = output.width / (float) output.height;

    Material      ivory(glm::dvec3(0.4, 0.4, 0.3),0.6, 0.1, 0.0,1.0);
    Material      glass(glm::dvec3(0.6, 0.7, 0.8),0.0, 0.1,  0.8,1.5);
    Material      red_rubber(glm::dvec3(0.3, 0.1, 0.1),0.9, 0.0, 0.0,1.0);
    Material      mirror(glm::dvec3(1.0, 1.0, 1.0),0.0, 0.8, 0.0,1.0);

    std::vector<Hittable *> sceneObjects;
    sceneObjects.push_back(new Sphere(glm::dvec3(-3,    0,   -16), 2,      ivory));
    sceneObjects.push_back(new Sphere(glm::dvec3(-1.0, -1.5, -12), 2,      glass));
    sceneObjects.push_back(new Sphere(glm::dvec3( 1.5, -0.5, -18), 3, red_rubber));
    sceneObjects.push_back(new Sphere(glm::dvec3( 7,    5,   -18), 4,     mirror));

    std::vector<PointLight> lights;
    lights.push_back(PointLight(glm::dvec3(-20, 20,  20), glm::dvec3(1, 1, 1)));
    lights.push_back(PointLight(glm::dvec3(30, 50, -25), glm::dvec3(1, 1, 1)));
    lights.push_back(PointLight(glm::dvec3(30, 20,  30), glm::dvec3(1, 1, 1)));
    for (int y = 0; y < output.height; ++y) {
        for (int x = 0; x < output.width; ++x) {
            // Convert the raster coordinates to [-1,1] range considering the aspect ratio and invert the Y coordinate
            glm::dvec2 screenCoord = glm::vec2((((x + 0.5) / output.width) * 2) - 1, (((output.height - y + 0.5) / output.height) * 2) - 1);
            // Consider the camera field of view and focal length/distance
            glm::dvec2 cameraCoord = screenCoord * tan(focalLength * (fieldOfView / 2.0) * (PI / 180));
            Ray r = Ray(glm::dvec3(0, 0, 0), glm::normalize(glm::dvec3(cameraCoord.x * aspectRatio, cameraCoord.y, -1)));
            glm::dvec3 color = glm::clamp(traceRay(r, sceneObjects, lights, 0), 0.0, 1.0) * 255.0;
            output.pixel(x, y, color);

        }
    }
    output.write(OUTPUT_PATH + "teste.png");
    return 0;
}
