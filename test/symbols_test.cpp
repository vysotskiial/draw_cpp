#include <gtest/gtest.h>
#include <formula_processor.h>
#include <string>
#include <cmath>
#include <print>

using namespace std::string_literals;

TEST(test, basic_arithmetic)
{
	FormulaProcessor pr_sum{"x1 + x2"};
	EXPECT_DOUBLE_EQ(pr_sum({1, 2}), 3.);

	FormulaProcessor pr_diff("x1 - x2");
	EXPECT_DOUBLE_EQ(pr_diff({3, 5}), -2.);

	FormulaProcessor pr_mult("x1 * x2");
	EXPECT_DOUBLE_EQ(pr_mult({3, 5}), 15.);

	FormulaProcessor pr_div("x1 / x2");
	EXPECT_DOUBLE_EQ(pr_div({3, 5}), 3 / 5.);

	FormulaProcessor pr_pow("x1 ^ x2");
	EXPECT_DOUBLE_EQ(pr_pow({3, 3}), 27);
}

TEST(test, brackets_arithmetic)
{
	FormulaProcessor pr_brackets{"(x1 + x2) * x3"};
	EXPECT_DOUBLE_EQ(pr_brackets({1, 2, 3}), 9.);
}

TEST(test, logic)
{
	FormulaProcessor pr_gr("x1 > x2");
	EXPECT_DOUBLE_EQ(pr_gr({3, 5}), 0);
	EXPECT_DOUBLE_EQ(pr_gr({5, 3}), 1);
	EXPECT_DOUBLE_EQ(pr_gr({5, 5}), 0);

	FormulaProcessor pr_ls("x1 < x2");
	EXPECT_DOUBLE_EQ(pr_ls({3, 5}), 1);
	EXPECT_DOUBLE_EQ(pr_ls({5, 3}), 0);
	EXPECT_DOUBLE_EQ(pr_ls({5, 5}), 0);

	FormulaProcessor pr_or("(x1 < x2) || (x1 > 2)");
	EXPECT_DOUBLE_EQ(pr_or({3, 5}), 1);
	EXPECT_DOUBLE_EQ(pr_or({5, 3}), 1);
	EXPECT_DOUBLE_EQ(pr_or({1, -1}), 0);

	FormulaProcessor pr_and("(x1 < x2) && (x1 > 2)");
	EXPECT_DOUBLE_EQ(pr_and({3, 5}), 1);
	EXPECT_DOUBLE_EQ(pr_and({5, 3}), 0);
	EXPECT_DOUBLE_EQ(pr_and({1, -1}), 0);

	FormulaProcessor pr_tern("(x1 < x2) ? x1 : x2");
	EXPECT_DOUBLE_EQ(pr_tern({3, 5}), 3);
	EXPECT_DOUBLE_EQ(pr_tern({5, 3}), 3);
	EXPECT_DOUBLE_EQ(pr_tern({1, -1}), -1);
}

TEST(test, functions)
{
	FormulaProcessor pr_power("x1 - x2^x3");
	EXPECT_DOUBLE_EQ(pr_power({3, 4, 1. / 2}), 1);
	FormulaProcessor pr_my_stuff("x2 - sign(x1)*|x1|^(0.25 + 0.25)");
	EXPECT_DOUBLE_EQ(pr_my_stuff({-22, 3}), 3 + std::sqrt(22.));
}

TEST(test, vector_test)
{
	VectorProcessor vp;
	vp[1] = "x2 - 10*sign(x1)*|x1|^0.5";
	vp[2] = "sign(x2 - 10*sign(x1)*|x1|^0.5) - 2.5*sign(x1)";
	auto vec = vp({16, 3});
	EXPECT_DOUBLE_EQ(vec[0], -37);
	EXPECT_DOUBLE_EQ(vec[1], -3.5);
}

TEST(test, aux_variable)
{
	VectorProcessor vp;
	vp[1] = "v1";
	vp[2] = "sign(v1) - 2.5 * sign(x1)";
	vp["v1"] = "x2 - 10*sign(x1)*|x1|^0.5";

	auto vec = vp({16, 3});
	EXPECT_EQ(vec.size(), 2);
	EXPECT_DOUBLE_EQ(vec[0], -37);
	EXPECT_DOUBLE_EQ(vec[1], -3.5);

	auto x1 = 0.;
	VectorProcessor error_p;
	error_p[1] = "x1p";
	error_p["x1p"] = "x1^(-0.5)";
	FormulaProcessor pr_pow_mul("x1 ^ (-0.5)");
	for (; x1 < 1.; x1 += 0.01) {
		EXPECT_DOUBLE_EQ(pr_pow_mul({x1}), pow(x1, -0.5));
		EXPECT_DOUBLE_EQ(error_p({x1})[0], pow(x1, -0.5));
	}
}

TEST(test, full)
{
	VectorProcessor vp;
	vp[1] = "av1";
	vp[2] = "sign(av1) - 2.5 * sign(x1+delta)";
	vp[3] = "delta";
	vp["av1"] = "x2 - 10*sign(x1+delta)*|x1+delta|^0.5";
	vp["delta"] = "(x1 > 1) ? -1 : -x1";

	auto vec = vp({17, 3});
	EXPECT_EQ(vec.size(), 3);
	EXPECT_DOUBLE_EQ(vec[0], -37);
	EXPECT_DOUBLE_EQ(vec[1], -3.5);
	EXPECT_DOUBLE_EQ(vec[2], -1);

	vec = vp({0.5, 5});
	EXPECT_EQ(vec.size(), 3);
	EXPECT_DOUBLE_EQ(vec[0], 5);
	EXPECT_DOUBLE_EQ(vec[1], 3.5);

	vp["delta"] = "delta + 1";
	EXPECT_THROW(vec = vp({0, 0}), std::runtime_error);
}

int main(int argc, char *argv[])
{
	/*
	FormulaProcessor pr_sum{"x1 + x2"s};
	std::println("sum of 1 and 2 is {}", pr_sum({1, 2}));
	return 0;
	*/
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
