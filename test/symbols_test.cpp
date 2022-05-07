#include <gtest/gtest.h>
#include <formula_processor.h>
#include <string>
#include <cmath>

using namespace std::string_literals;

TEST(test, basic_arithmetic)
{
	FormulaProcessor pr_sum{"x1 + x2"s, 2};
	EXPECT_DOUBLE_EQ(pr_sum({1, 2}), 3.);

	FormulaProcessor pr_diff("x1 - x2", 2);
	EXPECT_DOUBLE_EQ(pr_diff({3, 5}), -2.);

	FormulaProcessor pr_mult("x1 * x2", 2);
	EXPECT_DOUBLE_EQ(pr_mult({3, 5}), 15.);

	FormulaProcessor pr_div("x1 / x2", 2);
	EXPECT_DOUBLE_EQ(pr_div({3, 5}), 3 / 5.);
}

TEST(test, brackets_arithmetic)
{
	FormulaProcessor pr_brackets{"(x1 + x2) * x3", 3};
	EXPECT_DOUBLE_EQ(pr_brackets({1, 2, 3}), 9.);
}

TEST(test, functions)
{
	FormulaProcessor pr_power("x1 - pow(x2, x3)", 3);
	EXPECT_DOUBLE_EQ(pr_power({3, 4, 1. / 2}), 1);
	FormulaProcessor pr_my_stuff("x2 - sign(x1)*pow(abs(x1), 0.5)", 2);
	EXPECT_DOUBLE_EQ(pr_my_stuff({-22, 3}), 3 + std::sqrt(22.));
}

TEST(test, vector_test)
{
	VectorProcessor vp("x2 - 10*sign(x1)*pow(abs(x1), 0.5)\n"
	                   "sign(x2 - 10*sign(x1)*pow(abs(x1), 0.5)) - 2.5 * "
	                   "sign(x1)\n");
	auto vec = vp({16, 3});
	EXPECT_DOUBLE_EQ(vec[0], -37);
	EXPECT_DOUBLE_EQ(vec[1], -3.5);
}

TEST(test, aux_variable)
{
	VectorProcessor vp("v1 = x2 - 10*sign(x1)*pow(abs(x1), 0.5)\n"
	                   "v1\n"
	                   "sign(v1) - 2.5 * sign(x1)\n");

	auto vec = vp({16, 3});
	EXPECT_EQ(vec.size(), 2);
	EXPECT_DOUBLE_EQ(vec[0], -37);
	EXPECT_DOUBLE_EQ(vec[1], -3.5);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
