/*============================================================================

    (C) 2016 Martin Westerholt-Raum

    This file is part of HyCu.

    HyCu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    HyCu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with HyCu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

===============================================================================*/


#include <boost/test/unit_test.hpp>

#include <tuple>

#include <single_curve_fp.hh>


using namespace std;



BOOST_AUTO_TEST_CASE( fq_5_curve_1_2_3_1_1_0_4_opencl )
{
  auto curve = single_curve_fp(5, {1,2,3,1,1,0,4}, true);


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

BOOST_AUTO_TEST_CASE( fq_5_curve_1_2_3_1_1_0_4_cpu )
{
  auto curve = single_curve_fp(5, {1,2,3,1,1,0,4}, false);


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
