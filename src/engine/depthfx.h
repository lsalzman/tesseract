// eye space depth texture for soft particles, done at low res then blurred to prevent ugly jaggies
VARP(depthfxfpscale, 1, 1<<12, 1<<16);
VARP(depthfxscale, 1, 1<<6, 1<<8);
VARP(depthfxblend, 1, 16, 64);
VARP(depthfxpartblend, 1, 8, 64);
VAR(depthfxmargin, 0, 16, 64);
VAR(depthfxbias, 0, 1, 64);

extern void cleanupdepthfx();
VARFP(fpdepthfx, 0, 0, 1, cleanupdepthfx());
VARP(depthfxemuprecision, 0, 1, 1);
VARFP(depthfxsize, 6, 7, 12, cleanupdepthfx());
VARP(depthfx, 0, 1, 1);
VARP(depthfxparts, 0, 1, 1);
VARFP(depthfxrect, 0, 0, 1, cleanupdepthfx());
VARFP(depthfxfilter, 0, 1, 1, cleanupdepthfx());
VARP(blurdepthfx, 0, 1, 7);
VARP(blurdepthfxsigma, 1, 50, 200);
VAR(depthfxscissor, 0, 2, 2);
VAR(debugdepthfx, 0, 0, 1);

