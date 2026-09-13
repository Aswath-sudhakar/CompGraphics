
#include "doctest.h"
#include "Vec3.h"

TEST_CASE("Vec3 addition combines components correctly") {
    Vec3 a(1, 2, 3);
    Vec3 b(4, 5, 6);
    Vec3 result = a + b;
    CHECK(result.x == 5);
    CHECK(result.y == 7);
    CHECK(result.z == 9);
}

TEST_CASE("Dot product of perpendicular vectors is zero") {
    Vec3 a(1, 0, 0);
    Vec3 b(0, 1, 0);
    CHECK(a.dot(b) == doctest::Approx(0.0f));
}

TEST_CASE("Cross product of unit x and y axes gives unit z axis") {
    Vec3 x(1, 0, 0);
    Vec3 y(0, 1, 0);
    Vec3 z = x.cross(y);
    CHECK(z == Vec3(0, 0, 1));
}

TEST_CASE("Length of a 3-4-0 vector is 5 (Pythagorean triple)") {
    Vec3 v(3, 0, 4);
    CHECK(v.length() == doctest::Approx(5.0f));
}

TEST_CASE("Normalizing a zero vector throws") {
    Vec3 v(0, 0, 0);
    CHECK_THROWS_AS(v.normalized(), std::runtime_error);
}