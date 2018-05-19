/***************************************************************************
 *   Copyright (c) 2012 Juergen Riegel <FreeCAD@juergen-riegel.net>        *
 *   Copyright (c) 2013 Stefan Tröger  <stefantroeger@gmx.net>             *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development m_solvertem.         *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "../../App/PreCompiled.h"
#ifndef _PreComp_
#endif

#include <math.h>

#include <Standard_Failure.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <gp_Pln.hxx>
#include <GeomAbs_CurveType.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Pnt.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Cylinder.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <TopoDS_Face.hxx>

#include <Base/Placement.h>
#include <Base/Console.h>

#include "AFP.h"
#include "./AFP/AFPPy.h"
//#include "Item.h"
//#include "Product.h"

using namespace PartDesign;

namespace PartDesign {

	struct AFPInitException : std::exception {
		const char* what() const throw() {
			return "AFP cout not be initialised: unsoported geometry";
		}
	};
	struct AFPPartException : std::exception {
		const char* what() const throw() {
			return "AFP cout not be initialised: parts are invalid";
		}
	};
	struct AFPLinkException : std::exception {
		const char* what() const throw() {
			return "AFP cout not be initialised: unsoported link m_type";
		}
	};

	PROPERTY_SOURCE(PartDesign::AFP, App::DocumentObject)

	const char* AFP::m_orientationEnums[] = { "Parallel", "Equal", "Opposite", "Perpendicular", NULL };
	const char* AFP::m_typeEnums[] = { "AFP-VertexVertex", "AFP-VertexEdge", "AFP-VertexFace", "AFP-EdgeEdge", "AFP-EdgeFace", "AFP-FaceFace", NULL };
	const char* AFP::m_constraintEnums[] = { "Contact", "Coincident", "Distance", "Offset", "Angle", "None", NULL };
	const char* AFP::m_solutionSpaceEnums[] = { "Bidirectional", "PositivDirectional", "NegativeDirectional", NULL };

	AFP::AFP()
	{
		ADD_PROPERTY(m_first, (0));
		ADD_PROPERTY(m_second, (0));
		ADD_PROPERTY(m_value, (0));
		ADD_PROPERTY(m_orientation, (long(0)));
		m_orientation.setEnums(m_orientationEnums);
		ADD_PROPERTY(m_type, (long(6)));
		m_type.setEnums(m_typeEnums);
		ADD_PROPERTY(m_constraint, (long(6)));
		m_constraint.setEnums(m_constraintEnums);
		ADD_PROPERTY(m_solutionSpace, (long(0)));
		m_solutionSpace.setEnums(m_solutionSpaceEnums);
		Base::Uuid id;
		ADD_PROPERTY_TYPE(m_id, (""), 0, App::Prop_None, "ID (Part-Number) of the AFP");
		ADD_PROPERTY_TYPE(m_uuid, (id), 0, App::Prop_None, "UUID of the AFP");
	}

	short AFP::mustExecute() const
	{
		//if (Sketch.isTouched() ||
		//    Length.isTouched())
		//    return 1;
		return 0;
	}

	App::DocumentObjectExecReturn* AFP::execute(void)
	{
		return App::DocumentObject::StdReturn;
	}


	PyObject* AFP::getPyObject(void)
	{
		if (PythonObject.is(Py::_None())) {
			// ref counter is set to 1
			PythonObject = Py::Object(new AFPPy(this), true);
		}

		return Py::new_reference_to(PythonObject);
	}
}