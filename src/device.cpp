#include <Rcpp.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>
#include <R_ext/GraphicsEngine.h>

#include "agg_basics.h"
#include "agg_pixfmt_rgb.h"
#include "agg_renderer_base.h"
#include "agg_ellipse.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"

#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"

using namespace Rcpp;

#define R_USE_PROTOTYPES 1

typedef agg::pixfmt_rgb24                     pixfmt_type;
typedef agg::renderer_base<agg::pixfmt_rgb24> renbase_type;
enum { bytes_per_pixel = 3 };

class AggDevice {
public:
  renbase_type renderer;
  pixfmt_type* pixf;
  agg::rendering_buffer rbuf;
  unsigned char* buffer;
  int width;
  int height;
  int pageno;
  const char* file;
  agg::rgba8 background;
  AggDevice(const char* fp, int w, int h, int bg) {
    width = w;
    height = h;
    buffer = new unsigned char[width * height * bytes_per_pixel];
    rbuf = agg::rendering_buffer(buffer, width, height, width * bytes_per_pixel);
    pixf = new pixfmt_type(rbuf);
    renderer = renbase_type(*pixf);
    pageno = 0;
    file = fp;
    background = convertColour(bg);
    renderer.clear(background);
  }
  ~AggDevice() {
    delete pixf;
    delete [] buffer;
  }
  void newPage() {
    if (pageno != 0) {
      if (!savePage()) {
        stop("agg could not write to the given file");
      }
    }
    renderer.reset_clipping(true);
    renderer.clear(background);
    pageno++;
  }
  void close() {
    if (pageno == 0) pageno++;
    if (!savePage()) {
      stop("agg could not write to the given file");
    }
  }
  bool savePage() {
    char buf[PATH_MAX+1];
    snprintf(buf, PATH_MAX, file, pageno); buf[PATH_MAX] = '\0';
    FILE* fd = fopen(buf, "wb");
    if(fd)
    {
      fprintf(fd, "P6 %d %d 255 ", width, height);
      fwrite(buffer, 1, width * height * 3, fd);
      fclose(fd);
      return true;
    }
    return false;
  }
  void drawCircle(double x, double y, double r, int fill, int col, double lwd, int lty) {
    bool draw_fill = visibleColour(fill);
    bool draw_stroke = visibleColour(col) && lwd > 0.0;
    
    if (!draw_fill && !draw_stroke) return; // Early exit
    
    agg::rasterizer_scanline_aa<> pf;
    agg::scanline_p8 sl;
    agg::ellipse e1;
    e1.init(x, y, r, r);
    pf.add_path(e1);
    if (draw_fill) {
      agg::render_scanlines_aa_solid(pf, sl, renderer, convertColour(fill));
    }
    if (draw_stroke) {
      // TODO: draw outline
    }
  }
  void drawLine(double x1, double y1, double x2, double y2, int col, double lwd, int lty) {
    if (!visibleColour(col) || lwd == 0.0) return;
    agg::scanline_u8 sl;
    agg::rasterizer_scanline_aa<> ras;
    agg::path_storage ps;
    agg::conv_stroke<agg::path_storage> pg(ps);
    pg.width(lwd);
    ps.remove_all();
    ps.move_to(x1, y1);
    ps.line_to(x2, y2);
    ras.add_path(pg);
    agg::render_scanlines_aa_solid(ras, sl, renderer, convertColour(col));
  }
  
private:
  agg::rgba8 convertColour(unsigned int col) {
    return agg::rgba8(R_RED(col), R_GREEN(col), R_BLUE(col), R_ALPHA(col));
  }
  bool visibleColour(unsigned int col) {
    return col != NA_INTEGER && R_ALPHA(col) != 0;
  }
};

void agg_metric_info(int c, const pGEcontext gc, double* ascent,
                      double* descent, double* width, pDevDesc dd) {
  *ascent = 0.0;
  *descent = 0.0;
  *width = 0.0;
}

void agg_clip(double x0, double x1, double y0, double y1, pDevDesc dd) {
  AggDevice * device = (AggDevice *) dd->deviceSpecific;
  x0 = ceil(x0);
  x1 = floor(x1);
  y0 = ceil(y0);
  y1 = floor(y1);
  device->renderer.clip_box(x0, y0, x1, y1);
}

void agg_new_page(const pGEcontext gc, pDevDesc dd) {
  AggDevice * device = (AggDevice *) dd->deviceSpecific;
  device->newPage();
  return;
}

void agg_close(pDevDesc dd) {
  AggDevice * device = (AggDevice *) dd->deviceSpecific;
  device->close();
  delete device;
  return;
}

void agg_line(double x1, double y1, double x2, double y2,
               const pGEcontext gc, pDevDesc dd) {
  AggDevice * device = (AggDevice *) dd->deviceSpecific;
  device->drawLine(x1, y1, x2, y2, gc->col, gc->lwd, gc->lty);
  return;
}

void agg_polyline(int n, double *x, double *y, const pGEcontext gc,
                   pDevDesc dd) {
  return;
}
void agg_polygon(int n, double *x, double *y, const pGEcontext gc,
                  pDevDesc dd) {
  return;
}

void agg_path(double *x, double *y,
               int npoly, int *nper,
               Rboolean winding,
               const pGEcontext gc, pDevDesc dd) {
  return;
}

double agg_strwidth(const char *str, const pGEcontext gc, pDevDesc dd) {
  return 0.0;
}

void agg_rect(double x0, double y0, double x1, double y1,
               const pGEcontext gc, pDevDesc dd) {
  return;
}

void agg_circle(double x, double y, double r, const pGEcontext gc,
                 pDevDesc dd) {
  AggDevice * device = (AggDevice *) dd->deviceSpecific;
  device->drawCircle(x, y, r, gc->fill, gc->col, gc->lwd, gc->lty);
  return;
}

void agg_text(double x, double y, const char *str, double rot,
               double hadj, const pGEcontext gc, pDevDesc dd) {
  return;
}

void agg_size(double *left, double *right, double *bottom, double *top,
               pDevDesc dd) {
  *left = dd->left;
  *right = dd->right;
  *bottom = dd->bottom;
  *top = dd->top;
}

void agg_raster(unsigned int *raster, int w, int h,
                 double x, double y,
                 double width, double height,
                 double rot,
                 Rboolean interpolate,
                 const pGEcontext gc, pDevDesc dd) {
  return;
}


pDevDesc agg_device_new(AggDevice* device, int width, int height, double pointsize, int bg) {
  
  pDevDesc dd = (DevDesc*) calloc(1, sizeof(DevDesc));
  if (dd == NULL)
    return dd;
  
  dd->startfill = bg;
  dd->startcol = R_RGB(0, 0, 0);
  dd->startps = pointsize;
  dd->startlty = LTY_SOLID;
  dd->startfont = 1;
  dd->startgamma = 1;
  
  // Callbacks
  dd->activate = NULL;
  dd->deactivate = NULL;
  dd->close = agg_close;
  dd->clip = agg_clip;
  dd->size = agg_size;
  dd->newPage = agg_new_page;
  dd->line = agg_line;
  dd->text = agg_text;
  dd->strWidth = agg_strwidth;
  dd->rect = agg_rect;
  dd->circle = agg_circle;
  dd->polygon = agg_polygon;
  dd->polyline = agg_polyline;
  dd->path = agg_path;
  dd->mode = NULL;
  dd->metricInfo = agg_metric_info;
  dd->cap = NULL;
  dd->raster = agg_raster;
  
  // UTF-8 support
  dd->wantSymbolUTF8 = (Rboolean) 1;
  dd->hasTextUTF8 = (Rboolean) 1;
  dd->textUTF8 = agg_text;
  dd->strWidthUTF8 = agg_strwidth;
  
  // Screen Dimensions in pts
  dd->left = 0.0;
  dd->top = 0.0;
  dd->right = width;
  dd->bottom = height;
  
  // Magic constants copied from other graphics devices
  // nominal character sizes in pts
  dd->cra[0] = 0.9 * 12;
  dd->cra[1] = 1.2 * 12;
  // character alignment offsets
  dd->xCharOffset = 0.4900;
  dd->yCharOffset = 0.3333;
  dd->yLineBias = 0.2;
  // inches per pt
  dd->ipr[0] = 1.0 / 72.0;
  dd->ipr[1] = 1.0 / 72.0;
  
  // Capabilities
  dd->canClip = TRUE;
  dd->canHAdj = 2;
  dd->canChangeGamma = FALSE;
  dd->displayListOn = FALSE;
  dd->haveTransparency = 2;
  dd->haveTransparentBg = 2;
  
  dd->deviceSpecific = device;
  
  return dd;
}

void makeDevice(AggDevice* device, int width, int height, double pointsize, int bg) {
  R_GE_checkVersionOrDie(R_GE_version);
  R_CheckDeviceAvailable();
  BEGIN_SUSPEND_INTERRUPTS {
    pDevDesc dev = agg_device_new(device, width, height, pointsize, bg);
    if (dev == NULL)
      Rf_error("agg device failed to open");
    
    pGEDevDesc dd = GEcreateDevDesc(dev);
    GEaddDevice2(dd, "devAGG");
    GEinitDisplayList(dd);
    
  } END_SUSPEND_INTERRUPTS;
}

//[[Rcpp::export]]
SEXP agg_dev(String file, int width, int height, double pointsize, SEXP bg) {
  int bgCol = RGBpar(bg, 0);
  AggDevice* device = new AggDevice(file.get_cstring(), width, height, bgCol);
  makeDevice(device, width, height, pointsize, bgCol);
  
  return R_NilValue;
}