#define BOOST_TEST_MODULE SmallCurves
#include <boost/test/unit_test.hpp>

#include <tuple>

#include <single_curve_fp.hh>



BOOST_AUTO_TEST_CASE( fq_5_curve_1_2_3_1_1_0_4 )
{
  auto curve = single_curve_fp(5, {1,2,3,1,1,0,4});


  auto nmb_points = curve->number_of_points();
  BOOST_CHECK_MESSAGE(
      nmb_points[1] == make_tuple(6,0),
      "F_5 points: " << get<0>(nmb_points[1]) << " " << get<1>(nmb_points[1]) );
  BOOST_CHECK_MESSAGE(
      nmb_points[2] == make_tuple(40,2),
      "F_25 points: " << get<0>(nmb_points[2]) << " " << get<1>(nmb_points[2]) );


  auto hasse_weil_offsets = curve->hasse_weil_offsets(); 
  BOOST_CHECK_MESSAGE(
      hasse_weil_offsets[1] == 0,
      "Hasse Weil a1=" << hasse_weil_offsets[1] );

  BOOST_CHECK_MESSAGE(
      hasse_weil_offsets[2] == -16,
      "Hasse Weil a2=" << hasse_weil_offsets[2] );
}
