#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>


#include <VolumeViz/misc/SoVolumeDataPage.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>

SoVolumeDataPage::SoVolumeDataPage()
{
  this->size = SbVec2s(0, 0);
  this->format = 0;
  this->storage = NOT_LOADED;
  this->lastuse = 0;
  this->transferFunctionId = 0;
  this->data = NULL;
  this->palette = NULL;
  this->paletteFormat = 0;
  this->paletteSize = 0;
  this->nextPage = NULL;
}// constructor


SoVolumeDataPage::~SoVolumeDataPage()
{
  glDeleteTextures(1, &textureName);
  delete [] data;
  delete [] palette;
  delete nextPage;
  nextPage = NULL;
}// destructor


// FIXME: Some magic has to be done to make this one work with OpenGL 1.0. 
// torbjorv 08052002
void SoVolumeDataPage::setActivePage(long tick)
{
  glBindTexture(GL_TEXTURE_2D, this->textureName);
  this->lastuse = tick;

  int err = glGetError();
}// setActivePage



/*
If no palette specified, this function assumes RGBA data. If a palette
is specified, the input data should be indexes into the palette. 
The function uses the palette's size to decide whether the indices are
byte or short. 
*/
void SoVolumeDataPage::setData( Storage storage,
                                unsigned char * bytes,
                                const SbVec2s & size,
                                const float * palette,
                                int paletteFormat,
                                int paletteSize)
{
  this->size = size;
  this->storage = storage;
  this->lastuse = 0;
  this->paletteFormat = paletteFormat;
  this->paletteSize = paletteSize;

  // Possible creating an in-memory copy of all data
  if (storage & MEMORY) {
    this->data = new unsigned char[size[0]*size[1]*4];
    memcpy(this->data, bytes, size[0]*size[1]*4);

    if (palette != NULL) {
      this->palette = new unsigned char[sizeof(unsigned char)*paletteSize];
      memcpy(this->palette, palette, sizeof(float)*paletteSize);
    }// if
  }// if
  else {
    this->data = NULL;
    this->palette = NULL;
  }// else

  if (storage & OPENGL) {
    //FIXME: these functions is only supported in opengl 1.1... torbjorv 08052002
//    glEnable(GL_TEXTURE_2D);
//    glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
    glGenTextures(1, &this->textureName);
    glBindTexture(GL_TEXTURE_2D, this->textureName);


    // Uploading standard RGBA-texture
    if (palette == NULL) {
      glTexImage2D( GL_TEXTURE_2D, 
                    0,
                    4,
                    size[0], 
                    size[1],
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    bytes);
    }// if

    // Uploading paletted texture
    else {

      int err;

      switch (paletteSize) {
        case   2: __asm nop
                  break;
        case   4: __asm nop
                  break;
        case  16: __asm nop
                  break;
        case 256: __asm nop
                  break;
        default:  __asm nop
                  break;
      }// switch

      glColorTableEXT(GL_TEXTURE_2D, 
                      GL_RGBA, 
                      paletteSize,
                      GL_RGBA,
                      GL_FLOAT,
                      palette);

      err = glGetError();


      int actualPaletteSize;
      glGetColorTableParameterivEXT(GL_TEXTURE_2D, 
                                    GL_COLOR_TABLE_WIDTH_EXT, 
                                    &actualPaletteSize);

      int internalFormat;
      int format = GL_UNSIGNED_BYTE;
      switch (actualPaletteSize) {
        case   2: internalFormat = GL_COLOR_INDEX1_EXT;
                  break;
        case   4: internalFormat = GL_COLOR_INDEX2_EXT;
                  break;
        case  16: internalFormat = GL_COLOR_INDEX4_EXT;
                  break;
        case 256: internalFormat = GL_COLOR_INDEX8_EXT;
                  break;
        default:  internalFormat = GL_COLOR_INDEX16_EXT;
                  format = GL_UNSIGNED_SHORT;
                  break;
      }// switch

      glTexImage2D(GL_TEXTURE_2D, 
                   0,
                   internalFormat,
                   size[0],
                   size[1],
                   0,
                   GL_COLOR_INDEX,
                   format,
                   bytes);

      err = glGetError();

    }//else


    // FIXME: Okay. I tried. This GLWrapper-thingy must be spawned right 
    // out of hell. I'm really not able to compile it. But these lines 
    // need to be fixed for OpenGL 1.0 support. torbjorv 03082002

    // GLenum clamping;
    // const GLWrapper_t * glw = GLWRAPPER_FROM_STATE(state);
    // if (glw->hasTextureEdgeClamp) 
    //   clamping = GL_CLAMP_TO_EDGE;
    // else
    //   (GLenum) GL_CLAMP;


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  }// if  
  else
    this->textureName = 0;

}//setData

