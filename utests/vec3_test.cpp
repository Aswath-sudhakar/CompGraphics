#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include "vec3.h"

TEST_CASE("vec3 addition combines components correctly")
{
  vec3 a(1, 2, 3);
  vec3 b(4, 5, 6);
  vec3 result = a + b;
  REQUIRE(result.x() == 5);
  REQUIRE(result.y() == 7);
  REQUIRE(result.z() == 9);
}

TEST_CASE("dot product of perpendicular vectors is zero")
{
  vec3 a(1, 0, 0);
  vec3 b(0, 1, 0);
  REQUIRE(dot(a, b) == Catch::Approx(0.0));
}

TEST_CASE("cross product of unit x and y axes gives unit z axis")
{
  vec3 x(1, 0, 0);
  vec3 y(0, 1, 0);
  vec3 z = cross(x, y);
  REQUIRE(z.x() == Catch::Approx(0.0));
  REQUIRE(z.y() == Catch::Approx(0.0));
  REQUIRE(z.z() == Catch::Approx(1.0));
}

TEST_CASE("length of a 3-4-0 vector is 5 (Pythagorean triple)")
{
  vec3 v(3, 0, 4);
  REQUIRE(v.length() == Catch::Approx(5.0));
}

TEST_CASE("unit_vector produces a vector of length 1")
{
  vec3 v(3, 0, 4);
  vec3 u = unit_vector(v);
  REQUIRE(u.length() == Catch::Approx(1.0));
  // and it should point the same direction: components scaled by 1/5
  REQUIRE(u.x() == Catch::Approx(3.0 / 5.0));
  REQUIRE(u.z() == Catch::Approx(4.0 / 5.0));
}