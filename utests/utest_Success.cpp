#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE( "Simple Success Test" )
{
    int i = 42;
    REQUIRE( i == 42 );

    float myCalcOfPi = 3.141f;

    float eps = 1.0e-3; // a tolerance for how close the answer needs to be
    float actualAnswer = 3.14159f;
    REQUIRE_THAT( myCalcOfPi, Catch::Matchers::WithinAbs(actualAnswer, eps));
}
