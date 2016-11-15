#pragma once
#include <UT/UT_Vector2Array.h>
#include <GA/GA_AttributeRefMap.h>
#include <GU/GU_Detail.h>

struct BBox2D
{
	UT_Vector2R minvec;
	UT_Vector2R maxvec;
};

enum class ElementTypes {
	STRIPE  = 0x0001,
	STRIPE2 = 0x0002,
	STRIPE3 = 0x0004,
	TSHAPE  = 0x0008,
	RSHAPE  = 0x0010,
	SQUARE  = 0x0020,
	TRIANGLE = 0x0030,
};


class SubElem
{
public:
	UT_Vector2RArray coords;
};

class Element
{
public:
	Element(ElementTypes type, const short &direction, GA_Attribute *uvattr, GA_PrimitiveGroup *elem_grp, GA_PrimitiveGroup *elem_front_grp);
	~Element();
	BBox2D bbox();
	UT_Vector2R pivot();
	UT_Vector2R bounds_intersection();
	void append(SubElem elem);
	void transform(const UT_Vector2R &new_pos, const fpreal &scale, const bool flip);
	void build(GU_Detail *gdp, const GEO_Primitive *prim, const UT_Vector3 &primN, const fpreal &height);

	bool unwrapuvs;
	bool inherit_prim_attrs;
	GA_AttributeRefMap prim_refmap;

private:
	ElementTypes type;
	short direction;
	UT_ValArray<SubElem> subelements;
	GA_PrimitiveGroup *elem_group;
	GA_PrimitiveGroup *elem_front_group;
	GA_Attribute *uvattr;
	void move_by_vec(const UT_Vector2R &vec);
	exint num_points();

};

std::unique_ptr<Element> make_element(const ElementTypes &elem_type, 
									  const short &dir, 
									  bool inherit_prim_attrs,
						              bool unwrapuvs,
									  GA_AttributeRefMap &prim_refmap,
									  GA_Attribute *uvattr,
									  GA_PrimitiveGroup *elem_grp,
									  GA_PrimitiveGroup *elem_front_grp);

