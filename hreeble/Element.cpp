#include "Element.h"
#include <GEO/GEO_PrimPoly.h>
#include <UT/UT_Pair.h>
#include <SYS/SYS_Math.h>
#include <GA/GA_AttributeRefMap.h>
#include <GA/GA_ElementWrangler.h>

typedef UT_Vector2R V2R;
typedef UT_Pair<GA_Offset, GA_Offset> OffsetPair;

Element::Element(ElementTypes type, const short &direction, GA_Attribute *uvattr, GA_PrimitiveGroup *elem_grp, GA_PrimitiveGroup *elem_front_grp)
	:type(type), direction(direction), elem_group(elem_grp), elem_front_group(elem_front_grp), uvattr(uvattr)
{
}


Element::~Element()
{
}


BBox2D Element::bbox()
{
	fpreal min_s = subelements(0).coords(0).x();
	fpreal max_s = 0.0;
	fpreal min_t = subelements(0).coords(0).y();
	fpreal max_t = 0.0;

	for (const auto &subelem : subelements) {
		for (const auto &pt : subelem.coords) {
			if (pt.x() > max_s)
				max_s = pt.x();
			else if (pt.x() < min_s)
				min_s = pt.x();

			if (pt.y() > max_t)
				max_t = pt.y();
			else if (pt.y() < min_t)
				min_t = pt.y();
	}
	}
	BBox2D bbox = { UT_Vector2R(min_s, min_t), UT_Vector2R(max_s, max_t) };
	return bbox;
}


UT_Vector2R Element::pivot()
{
	UT_Vector2R pivot(0.0, 0.0);
	for (const auto &elem: subelements) {
		for (const auto &pt : elem.coords) {
			pivot += pt;
		}
	}
	pivot /= num_points();
	return pivot;
}


UT_Vector2R Element::bounds_intersection()
{
	auto sign = [](const fpreal val) { return val > 0 ? 1 : 0; };
	auto outside = [](fpreal val) { return val > 1.0 || val < 0.0; };

	V2R offset(0.0, 0.0);
	BBox2D bbox = this->bbox();
	if (outside(bbox.minvec.x())) {
		offset += V2R(sign(bbox.minvec.x()) - bbox.minvec.x(), 0.0);
	}
	if (outside(bbox.minvec.y())) {
		offset += V2R(0.0, sign(bbox.minvec.y()) - bbox.minvec.y());
	}

	if (outside(bbox.maxvec.x())) {
		offset += V2R(sign(bbox.maxvec.x()) - bbox.maxvec.x(), 0.0);
	}
	if (outside(bbox.maxvec.y())) {
		offset += V2R(0.0, sign(bbox.maxvec.y()) - bbox.maxvec.y());

	}
	return offset;
}


void Element::append(SubElem elem)
{
	subelements.append(elem);
}


void Element::move_by_vec(const UT_Vector2R & vec)
{
	for (auto &subelem : subelements) {
		for (auto &pt : subelem.coords) {
			pt += vec;
		}
	}
}


void Element::transform(const UT_Vector2R & new_pos, const fpreal & scale, const bool flip)
{
	// Flip
	if (flip) {
		if (type == ElementTypes::TSHAPE || type == ElementTypes::RSHAPE) {
			for (auto &subelem : subelements) {
				subelem.coords.reverse();
				for (auto &pt : subelem.coords) {
					pt(direction) = 1 - pt(direction);
				}
			}
		}
	}
	// Move
	UT_Vector2R vec;
	if (type == ElementTypes::TRIANGLE) {
		UT_Vector2R pp;
		pp(0) = SYSfit01(new_pos(0), 0.0, 1 - new_pos(1));
		pp(1) = SYSfit01(new_pos(1), 0.0, 1 - new_pos(0));
		vec = pp - this->pivot();
	}
	else
		vec = new_pos - this->pivot();
	move_by_vec(vec);
	UT_Vector2R pivot = this->pivot();
	// Scale
	for (auto &subelem : subelements) {
		for (auto &pt : subelem.coords) {
			pt += (pt - pivot) * (scale - 1);
		}
	}
	// Place
	V2R offset = bounds_intersection();
	if (offset.length() != 0) {
		offset *= 1.2;
		move_by_vec(offset);
	}
	//if (bounds_intersection().length() != 0) {
	//	for (auto &subelem : subelements) {
	//		for (int i = 0; i < subelem.coords.entries(); i++) {
	//			auto coord = subelem.coords(i);
	//			if (SYSalmostEqual(coord.x(), 1.0)) {
	//				
	//			}
	//		}
	//	}
	//}
	if (bounds_intersection().length() != 0) {
		for (auto &subelem : subelements) {
			for (auto &pt : subelem.coords) {
				pt(0) = SYSmax(SYSmin(pt.x(), 0.99), 0.01);
				pt(1) = SYSmax(SYSmin(pt.y(), 0.99), 0.01);
			}
		}
	}
}



void Element::build(GU_Detail *gdp, const GEO_Primitive *prim, const UT_Vector3F &primN, const fpreal & height)
{
	GA_RWHandleV3 ph(gdp->getP());
	GA_AttributeRefMap vertex_refmap(*gdp);
	GA_RWHandleV3D vh;
	UT_Vector3R island_center;
	UT_Array<GEO_PrimPoly*> result_prims;
	fpreal uv_area, source_prim_area;
	if (this->unwrapuvs) {
		vh = uvattr;
		vertex_refmap.appendDest(vh.getAttribute());
		island_center.assign(0.0);
		for (GA_Iterator it(prim->getVertexRange()); !it.atEnd(); ++it) { island_center += vh.get(*it); }
		island_center /= prim->getVertexCount();
		island_center.z() = 0.0;
		// Calc source prim area and uv_area
		source_prim_area = prim->calcArea();
		UT_Vector3R v1 = vh.get(prim->getVertexOffset(1)) - vh.get(prim->getVertexOffset(0));
		UT_Vector3R v2 = vh.get(prim->getVertexOffset(3)) - vh.get(prim->getVertexOffset(0));
		uv_area = v1.length() * v2.length();
	}

	for (const auto &subelem : subelements) {
		exint num_coords = subelem.coords.entries();
		GA_Offset point_block = gdp->appendPointBlock(num_coords * 2);
		GA_OffsetArray top_ptoffs;
		top_ptoffs.clear();
		for (exint i = 0; i < num_coords; i++) {
			bool last(i == (num_coords - 1));
			const UT_Vector2R &coord0 = subelem.coords(i);
			const UT_Vector2R &coord1 = subelem.coords((last ? 0 : i + 1));
			UT_Vector4F pt0, pt1, pt2, pt3;
			GA_Offset ptof0 = point_block + i*2;
			GA_Offset ptof1 = point_block + i*2 + 1;
			GA_Offset ptof2 = point_block + (last ? 0 : i*2 + 2);
			GA_Offset ptof3 = point_block + (last ? 1 : i*2 + 3);
			top_ptoffs.append(ptof1);
			prim->evaluateInteriorPoint(pt0, coord0.x(), coord0.y());
			prim->evaluateInteriorPoint(pt1, coord1.x(), coord1.y());
			ph.set(ptof0, UT_Vector3F(pt0));
			ph.set(ptof2, UT_Vector3F(pt1));
			
			ph.set(ptof1, UT_Vector3F(pt0) + primN * height);
			ph.set(ptof3, UT_Vector3F(pt1) + primN * height);
			
			auto new_prim = GEO_PrimPoly::build(gdp, 4, false, false);
			result_prims.append(new_prim);
			new_prim->setVertexPoint(0, ptof0);
			new_prim->setVertexPoint(1, ptof2);
			new_prim->setVertexPoint(2, ptof3);
			new_prim->setVertexPoint(3, ptof1);
			if (this->unwrapuvs) {
				prim->evaluateInteriorPoint(new_prim->getVertexOffset(0), vertex_refmap, coord0.x(), coord0.y());
				prim->evaluateInteriorPoint(new_prim->getVertexOffset(1), vertex_refmap, coord1.x(), coord1.y());
				vertex_refmap.copyValue(GA_ATTRIB_VERTEX, new_prim->getVertexOffset(2), GA_ATTRIB_VERTEX, new_prim->getVertexOffset(1));
				vertex_refmap.copyValue(GA_ATTRIB_VERTEX, new_prim->getVertexOffset(3), GA_ATTRIB_VERTEX, new_prim->getVertexOffset(0));
				
				UT_Vector3R edge = vh.get(new_prim->getVertexOffset(1)) - vh.get(new_prim->getVertexOffset(0));
				fpreal uv_edge_len = edge.length();
				fpreal extruded_prim_area = new_prim->calcArea();
				fpreal uv_extruded_area = (extruded_prim_area * uv_area) / source_prim_area;
				fpreal offset_val = uv_extruded_area / uv_edge_len;
				UT_Vector3R vc = island_center - vh.get(new_prim->getVertexOffset(0));
				UT_Vector3R vv = vh.get(new_prim->getVertexOffset(1)) - vh.get(new_prim->getVertexOffset(0));
				fpreal proj = vc.dot(vv) / vv.length();
				vv.normalize();
				UT_Vector3R projpoint = vh.get(new_prim->getVertexOffset(0)) + vv * proj;
				UT_Vector3R offset_dir = projpoint - island_center;
				offset_dir.normalize();
				
				vh.add(new_prim->getVertexOffset(0), offset_dir * offset_val);
				vh.add(new_prim->getVertexOffset(1), offset_dir * offset_val);

			}
			if (elem_group != nullptr)
				elem_group->add(new_prim);
			
		}
		auto top_prim = GEO_PrimPoly::build(gdp, num_coords, false, false);
		result_prims.append(top_prim);
		for (int j = 0; j < num_coords; j++) {
			top_prim->setVertexPoint(j, top_ptoffs(j));
			auto vtxoff = top_prim->getVertexOffset(j);
			if (this->unwrapuvs)
				prim->evaluateInteriorPoint(vtxoff, vertex_refmap, subelem.coords(j).x(), subelem.coords(j).y());
		}
		if (inherit_prim_attrs) {
			for (auto const &each : result_prims) {
				prim_refmap.copyValue(GA_ATTRIB_PRIMITIVE, each->getMapOffset(), GA_ATTRIB_PRIMITIVE, prim->getMapOffset());
			}
		}
		if (elem_front_group != nullptr)
			elem_front_group->add(top_prim);
	}
}

exint Element::num_points()
{
	exint num = 0;
	for (const auto &elem : subelements) {
		num += elem.coords.entries();
	}
	return num;
}


std::unique_ptr<Element> 
make_element(const ElementTypes &elem_type, 
			const short &dir,
			bool inherit_prim_attrs,
			bool unwrapuvs,
			GA_AttributeRefMap &prim_refmap,
			GA_Attribute *uvattr,
			GA_PrimitiveGroup *elem_grp,
			GA_PrimitiveGroup *elem_front_grp) 
{
	std::unique_ptr<Element> w(new Element(elem_type, dir, uvattr, elem_grp, elem_front_grp));
	w->unwrapuvs = unwrapuvs;
	w->inherit_prim_attrs = inherit_prim_attrs;
	w->prim_refmap = prim_refmap;
	uint num_elems = 1;
	if (elem_type <= ElementTypes::STRIPE3)
	{
		switch (elem_type)
		{
		case ElementTypes::STRIPE2:
			num_elems = 2;
			break;
		case ElementTypes:: STRIPE3:
			num_elems = 3;
			break;
		default:
			break;
		}

		V2R pt0, pt1, pt2, pt3;
		if (dir == 0) {
			pt0 = V2R(0.0, 0.0);
			pt1 = V2R(0.0, 1.0);
			pt2 = V2R(0.1, 1.0);
			pt3 = V2R(0.1, 0.0);
		}
		else {
			pt0 = V2R(0.0, 0.0);
			pt1 = V2R(0.0, 0.1);
			pt2 = V2R(1.0, 0.1);
			pt3 = V2R(1.0, 0.0);
		}

		for (uint i = 0; i < num_elems; i++) {
			auto elem = SubElem();
			fpreal step = i / 8.0;
			auto new_pt0 = pt0;
			new_pt0(dir) += step;
			elem.coords.append(new_pt0);
			auto new_pt1 = pt1;
			new_pt1(dir) += step;
			elem.coords.append(new_pt1);
			auto new_pt2 = pt2;
			new_pt2(dir) += step;
			elem.coords.append(new_pt2);
			auto new_pt3 = pt3;
			new_pt3(dir) += step;
			elem.coords.append(new_pt3);
			w->append(elem);
		}
	}
	else if (elem_type == ElementTypes::TSHAPE)
	{
		auto elem = SubElem();
		if (dir == 0) {
			elem.coords.append(V2R(0.0, 0.0));
			elem.coords.append(V2R(0.0, 0.99));
			elem.coords.append(V2R(0.33, 0.99));
			elem.coords.append(V2R(0.33, 0.66));
			elem.coords.append(V2R(0.66, 0.66));
			elem.coords.append(V2R(0.66, 0.33));
			elem.coords.append(V2R(0.33, 0.33));
			elem.coords.append(V2R(0.33, 0.0));
		}
		else {
			elem.coords.append(V2R(0.0, 0.0));
			elem.coords.append(V2R(0.0, 0.33));
			elem.coords.append(V2R(0.33, 0.33));
			elem.coords.append(V2R(0.33, 0.66));
			elem.coords.append(V2R(0.66, 0.66));
			elem.coords.append(V2R(0.66, 0.33));
			elem.coords.append(V2R(0.99, 0.33));
			elem.coords.append(V2R(0.99, 0.0));
		}
		w->append(elem);
	}

	else if (elem_type == ElementTypes::SQUARE)
	{
		auto elem = SubElem();
		elem.coords.append(V2R(0.0, 0.0));
		elem.coords.append(V2R(0.0, 0.5));
		elem.coords.append(V2R(0.5, 0.5));
		elem.coords.append(V2R(0.5, 0.0));
		w->append(elem);
	}
	else if (elem_type == ElementTypes::RSHAPE)
	{
		auto elem = SubElem();
		if (dir == 0) {
			elem.coords.append(V2R(0.99, 0.33));
			elem.coords.append(V2R(0.99, 0.0));
			elem.coords.append(V2R(0.0, 0.0));
			elem.coords.append(V2R(0.0, 0.66));
			elem.coords.append(V2R(0.33, 0.66));
			elem.coords.append(V2R(0.33, 0.33));
		}
		else {
			elem.coords.append(V2R(0.33, 0.0));
			elem.coords.append(V2R(0.0, 0.00));
			elem.coords.append(V2R(0.0, 0.99));
			elem.coords.append(V2R(0.66, 0.99));
			elem.coords.append(V2R(0.66, 0.66));
			elem.coords.append(V2R(0.33, 0.66));

		}
		w->append(elem);
	}
	else if (elem_type == ElementTypes::TRIANGLE)
	{
		auto elem = SubElem();
		elem.coords.append(V2R(0.0, 0.0));
		elem.coords.append(V2R(0.0, 0.5));
		elem.coords.append(V2R(0.5, 0.0));
		w->append(elem);
	}
	return w;
}
