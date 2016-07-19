/*
 * @BEGIN LICENSE
 *
 * Psi4: an open-source quantum chemistry software package
 *
 * Copyright (c) 2007-2016 The Psi4 Developers.
 *
 * The copyrights for code used from other parties are included in
 * the corresponding files.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * @END LICENSE
 */

#include <boost/python.hpp>
#include <libcubeprop/cubeprop.h>
#include <liboptions/liboptions.h>
#include <libmints/wavefunction.h>

using namespace boost::python;
using namespace psi;

void export_cubeprop()
{
    class_<CubeProperties, boost::shared_ptr<CubeProperties> >("CubeProperties", "docstring", no_init)
        .def(init<boost::shared_ptr<Wavefunction>>())
        .def("compute_properties", &CubeProperties::compute_properties, "docstring");
}