import os
from waf_common import setup_houdini

HHOME = r'C:/Users/alex/Documents/houdini16.0'
DSO_HOME = os.path.join(HHOME, 'DSO')


def configure(conf):
    conf.setup_houdini()


def build(ctx):
	ctx.objects(source="hreeble\Element.cpp", 
				target="objects",
				includes=['hreeble', ctx.env.HFS_INC],
				defines=ctx.env.DEFINES)

	ctx.shlib(source="hreeble\sop_hreeble.cpp",
				target='hreeble',
				includes=['hreeble', ctx.env.HFS_INC],
				defines=ctx.env.DEFINES,
				use="objects")
	ctx.install_files(DSO_HOME, ['hreeble.dll'])
