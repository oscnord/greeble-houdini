import os
from waflib.Configure import conf

@conf
def setup_houdini(ctx):
    ctx.load('msvc')
    ctx.env.HFS = os.getenv("DEV_HFS")
    ctx.env.HFS_LIB = os.path.join(ctx.env.HFS, 'custom/houdini/dsolib')
    ctx.env.HFS_INC = os.path.join(ctx.env.HFS, 'toolkit/include')
    ctx.env.CXXFLAGS = ['-nologo', '-TP', '-Zc:forScope', '-wd4355', '-w14996', '-O2', '-MD', '-EHsc', '-GR',
                         '-bigobj']
    ctx.env.DEFINES = ['MAKING_DSO', 'NDEBUG', 'I386', 'WIN32', 'SWAP_BITFIELDS', '_WIN_32_WINNT=0X0502',
                        'NOMINMAX', 'STRICT', 'WIN32_LEAN_AND_MEAN', '_USE_MATH_DEFINES',
                        '_CRT_SECURE_NO_DEPRICATE', '_CRT_SECURE_NO_WARNINGS', '_CRT_NONSTDC_NO_DEPRECATE', '_SCL_SECURE_NO_WARNINGS',
                        'BOOST_ALL_NO_LIB', 'SESI_LITTLE_ENDIAN', 'AMD64', 'SIZEOF_VOID_P=8', 'FBX_ENABLED=1',
                        'OPENCL_ENABLED=1', 'OPENVDB_ENABLED=1']

    hlibs = ctx.env.HFS_LIB
    ctx.env.LDFLAGS = ['-LIBPATH:%s' % hlibs, "%s/*.a" % hlibs, '%s/*.lib' % hlibs]
