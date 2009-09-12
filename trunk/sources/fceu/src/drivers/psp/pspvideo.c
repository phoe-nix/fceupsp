#include <pspgu.h>
#include <pspdisplay.h>
#include <string.h>

#include "../../types.h"
#include "../../driver.h"
#include "../../fceu.h"
#include "vram.h"

extern uint8 *XBuf;

void swizzle_fast(u8* out, const u8* in, unsigned int width, unsigned int height);
void advancedBlit(int sx, int sy, int sw, int sh, int dx, int dy, int slice);

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

#define SLICE_SIZE 8 // change this to experiment with different page-cache sizes

u32 NesPalette[64] =
{
0x6c6c6c, 0x00268e, 0x0000a8, 0x400094, 0x700070, 0x780040, 0x700000, 0x621600, // 1
0x442400, 0x343400, 0x005000, 0x004444, 0x004060, 0x000000, 0x101010, 0x101010, // 2
0xbababa, 0x205cdc, 0x3838ff, 0x8020f0, 0xc000c0, 0xd01474, 0xd02020, 0xac4014, // 3
0x7c5400, 0x586400, 0x008800, 0x007468, 0x00749c, 0x202020, 0x101010, 0x101010, // 4
0xffffff, 0x4ca0ff, 0x8888ff, 0xc06cff, 0xff50ff, 0xff64b8, 0xff7878, 0xff9638, // 5
0xdbab00, 0xa2ca20, 0x4adc4a, 0x2ccca4, 0x1cc2ea, 0x585858, 0x101010, 0x101010, // 6
0xffffff, 0xb0d4ff, 0xc4c4ff, 0xe8b8ff, 0xffb0ff, 0xffb8e8, 0xffc4c4, 0xffd4a8, // 7
0xffe890, 0xf0f4a4, 0xc0ffc0, 0xacf4f0, 0xa0e8ff, 0xc2c2c2, 0x202020, 0x101010  // 8
};

struct Vertex
{
	float u,v;
	float x,y,z;
};

static unsigned int __attribute__((aligned(16))) list[262144];
unsigned int __attribute__((aligned(16))) clut256[256];
void* vram_buffer;

void PSPVideoInit() {
	// Setup GU
	sceGuInit(); // Turn on the GU
	sceGuStart(GU_DIRECT,list); // Start filling a command list.

	void* fbp0 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	void* fbp1 = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	void* zbp = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);

	vram_buffer = getStaticVramTexture(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);

	XBuf = (uint8 *)((unsigned int)vram_buffer|0x40000000);

	sceGuDrawBuffer(GU_PSM_8888,fbp0,BUF_WIDTH); // Point out the drawing buffer
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp1,BUF_WIDTH); // Point out the display buffer
	sceGuDepthBuffer(zbp,BUF_WIDTH); // Point out the depth buffer
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2)); // Define current drawing area.
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT); // Center screen in virtual space.
	sceGuDepthRange(0xc350,0x2710); // Tells the GU what value range to use within the depth buffer.
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT); // Sets up a scissor rect for the screen.
	sceGuEnable(GU_SCISSOR_TEST); // Enables scissor mode: pixels outside the scissor rect are not rendered.
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D); // Enables texturing of primitives.
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT); // Clears current drawbuffer

	sceGuFinish(); // End of command list
	sceGuSync(0,0); // Wait for list to finish executing

	sceDisplayWaitVblankStart(); // Wait for vertical blank start
	sceGuDisplay(GU_TRUE); // VRAM should be displayed on screen.

	// Clear screen
	//sceGuClearColor(0xff00ff); // Sets current clear color
	//sceGuClear(GU_COLOR_BUFFER_BIT); // Clears current drawbuffer
}


void PSPVideoRenderFrame(uint8 *XBuf) {
	sceGuStart(GU_DIRECT,list);

	//memcpy(vram_buffer,XBuf,256 * 256);
	//swizzle_fast((u8*)vram_buffer,(const u8*)XBuf,256,256);

	// setup CLUT texture
	sceGuClutMode(GU_PSM_8888,0,0xff,0); // 32-bit palette
	sceGuClutLoad((256/8),clut256); // upload 32*8 entries (256)
	sceGuTexMode(GU_PSM_T8,0,0,0); // 8-bit image
	sceGuTexImage(0,256,256,256,vram_buffer);
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGB);
	//sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	//sceGuTexScale(2.0f,2.0f);
	//sceGuTexOffset(0.0f,0.0f);
	//sceGuAmbientColor(0xffffffff);

//	// render sprite
//	sceGuColor(0xffffffff);
//	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
//	vertices[0].u = 0; vertices[0].v = 0;
//	vertices[0].x = 58; vertices[0].y = 0; vertices[0].z = 0;
//	vertices[1].u = 256; vertices[1].v = 240;
//	vertices[1].x = 422; vertices[1].y = 272; vertices[1].z = 0;
//	//vertices[1].x = 394; vertices[1].y = 290; vertices[1].z = 0;
//	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);

	advancedBlit(0, 0, 256, 240, 112, 16, SLICE_SIZE);

	// wait for next frame
	sceGuFinish();
	sceGuSync(0,0);

	//sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

void PSPVideoOverrideNESClut() {
	int i, r, g, b;
	unsigned int* clut = (unsigned int*)(((unsigned int)clut256)|0x40000000);
	//unsigned int* clut = clut256;
	unsigned int color;

	for(i = 0; i < 64 ; i++ ) {
        r = (NesPalette[i] & 0xff0000) >> 16;
        g = (NesPalette[i] & 0xff00) >> 8;
        b = (NesPalette[i] & 0xff) << 0;

        color = ((b<<16)|(g<<8)|(r<<0));

        clut[i]     = color;
        clut[i+64]  = color;
        clut[i+128] = color;
        clut[i+192] = color;
    }
}

void swizzle_fast(u8* out, const u8* in, unsigned int width, unsigned int height)
{
   unsigned int blockx, blocky;
   unsigned int j;

   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);

   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;

   const u8* ysrc = in;
   u32* dst = (u32*)out;

   for (blocky = 0; blocky < height_blocks; ++blocky)
   {
      const u8* xsrc = ysrc;
      for (blockx = 0; blockx < width_blocks; ++blockx)
      {
         const u32* src = (u32*)xsrc;
         for (j = 0; j < 8; ++j)
         {
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            src += src_pitch;
         }
         xsrc += 16;
     }
     ysrc += src_row;
   }
}

void advancedBlit(int sx, int sy, int sw, int sh, int dx, int dy, int slice)
{
	int start, end;

	// blit maximizing the use of the texture-cache

	for (start = sx, end = sx+sw; start < end; start += slice, dx += slice)
	{
		struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
		int width = (start + slice) < end ? slice : end-start;

		vertices[0].u = start; vertices[0].v = sy;
		//vertices[0].color = 0;
		vertices[0].x = dx; vertices[0].y = dy; vertices[0].z = 0;

		vertices[1].u = start + width; vertices[1].v = sy + sh;
		//vertices[1].color = 0;
		vertices[1].x = dx + width; vertices[1].y = dy + sh; vertices[1].z = 0;

		//sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}
}
