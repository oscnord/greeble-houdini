#pragma once
#include <UT/UT_DSOVersion.h>
#include <SOP/SOP_Node.h>
#include <OP/OP_Node.h>
#include <PRM/PRM_Include.h>
#include <GEO/GEO_PrimPoly.h>
#include <UT/UT_ValArray.h>


class SOP_Hreeble : public SOP_Node
{
public:
	SOP_Hreeble(OP_Network *net, const char *name, OP_Operator *op);
	~SOP_Hreeble();
	static OP_Node *creator(OP_Network*, const char*, OP_Operator*);
	virtual OP_ERROR cookInputGroups(OP_Context &ctx, int alone = 0);
	void split_primitive(GEO_Primitive *prim, UT_ValArray<GEO_Primitive*> &result, const unsigned short dir = 0);
	void divide(GEO_Primitive *prim, UT_ValArray<GEO_Primitive*> &result);
	GEO_Primitive* extrude(GEO_Primitive *prim, const fpreal &height, const fpreal &inset);
	static PRM_Template myparms[];

	GA_PrimitiveGroup *top_prims_grp;
	GA_PrimitiveGroup *work_group;

protected:
	virtual OP_ERROR cookMySop(OP_Context &ctx);
	bool updateParmsFlags();

private:
	uint SeedPRM() { return (uint)evalInt("seed", 0, 0); }
	void SourceGroupsPRM(UT_String &str) { evalString(str, "source_groups", 0, 0); }
	uint ElemDensityPRM() { return (uint)evalInt("elem_density", 0, 0); }
	uint GeneratePanelsPRM() { return (uint)evalInt("gen_panels", 0, 0); }
	fpreal64 PanelInsetPRM() { return evalFloat("panel_inset", 0, 0.0); }
	void PanelHeightPRM(fpreal64 vals[], const fpreal &time) { evalFloats("panel_height", vals, time); }
	void ElemScalePRM(fpreal64 vals[], const fpreal &time) { evalFloats("elem_scale", vals, time); }
	void ElemHeightPRM(fpreal64 vals[], const fpreal &time) { evalFloats("elem_height", vals, time); }
	uint SelectedShapesPRM() { return evalInt("elem_shapes", 0, 0); }
	uint DoConvexPRM() { return evalInt("convex", 0, 0); }
	uint CreateGroupsPRM() { return evalInt("elem_groups", 0, 0); }
	uint UnwrapUVsPRM() { return evalInt("unwrap_uvs", 0, 0); }
	uint InheritAttribsPRM() { return evalInt("inherit_attribs", 0, 0); }

	GA_RWHandleV3 phandle; // point handle
	GA_Attribute *uvattr; // vertext handle
	GA_AttributeRefMap prim_refmap;
	UT_ValArray<GEO_Primitive*> kill_prims;
	const GA_PrimitiveGroup *source_prim_group;
	GA_PrimitiveGroup *elements_group;
	GA_PrimitiveGroup *elements_front_group;
	uint inherit_attribs;
	uint unwrap_uvs;
	uint my_seed;
};