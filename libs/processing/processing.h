#ifndef PROCESSING_H
#define PROCESSING_H

#include <iostream>
#include <string>
#include <cstring>
#include <cstdint>
#include <vector>
#include <functional>
#include <csignal>

extern "C" {
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
}

#include <xf86drm.h>
#include <xf86drmMode.h>

#define VCARD "/dev/dri/card0"

namespace proc {

  struct buffer {
    public:
      uint32_t width;
      uint32_t height;
      uint32_t stride;
      uint32_t size;
      uint32_t handle;

      uint8_t * map;

      uint32_t frame_buffer;

      uint32_t & operator [] (uint32_t i) { return *(uint32_t*)&map[i]; }
  };

  struct device {
    public:
      uint32_t front_buffer = 0;

      buffer buffers[2];

      drmModeModeInfo mode;

      uint32_t connector;
      uint32_t crtc;

      drmModeCrtc * saved_crtc;

      bool page_flip_pending;
      bool cleanup;
  };

  namespace system {

    static void close_fd(int32_t fd) {
      close(fd);
    }

    static bool open_fd(int32_t & fd, std::string vcard = VCARD) { 
      uint64_t has_dumb;

      fd = open(vcard.c_str(), O_RDWR | O_CLOEXEC);

      if (fd < 0) {
        std::cout << "cannot open " << vcard << std::endl;
        return false;
      }

      if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 ||
          !has_dumb) {
        std::cout << "drm device does not support dumb buffers" << std::endl;

        close_fd(fd);

        return false;
      }

      return true;
    }

  }

  namespace event {

    static std::function<void(buffer & buf)> on_draw;

    static std::function<void(int32_t)> on_close;

    static void system_event(int32_t sig) {
      on_close(sig);

      std::_Exit(EXIT_SUCCESS);
    }

    static void draw_event(int32_t fd, device & dev) {
      buffer & buf = dev.buffers[dev.front_buffer ^ 1];

      on_draw(buf);

      if (drmModePageFlip(
            fd,
            dev.crtc,
            buf.frame_buffer,
            DRM_MODE_PAGE_FLIP_EVENT,
            &dev))
        std::cout << "cannot flip crtc for connector" <<
          dev.connector << std::endl;
      else {
        dev.front_buffer ^= 1;
        dev.page_flip_pending = true;
      }
    }

    static void flip_event(
        int32_t fd,
        uint32_t frame,
        uint32_t sec,
        uint32_t usec,
        void * data) {
      device & dev = *static_cast<device *>(data);

      dev.page_flip_pending = false;

      if (!dev.cleanup)
        draw_event(fd, dev);
    }

    static void dispatch(int32_t fd, device & dev) {
      std::signal(SIGINT, system_event);

      drmEventContext ev;

      std::memset(&ev, 0, sizeof(ev));

      ev.version = 2;
      ev.page_flip_handler = flip_event;

      draw_event(fd, dev);

      for (;;)
        drmHandleEvent(fd, &ev);
    }

  }

  namespace hardware {

    static void destroy_fb(int32_t fd, buffer & buf) {
      struct drm_mode_destroy_dumb dreq;

      munmap(buf.map, buf.size);

      drmModeRmFB(fd, buf.frame_buffer);

      std::memset(&dreq, 0, sizeof(dreq));

      dreq.handle = buf.handle;

      drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    }

    static bool create_fb(int32_t fd, buffer & buf) {
      struct drm_mode_create_dumb creq;
      struct drm_mode_destroy_dumb dreq;
      struct drm_mode_map_dumb mreq;

      auto error_fb = [&]() { 
        drmModeRmFB(fd, buf.frame_buffer);

        std::memset(&dreq, 0, sizeof(dreq));

        dreq.handle = buf.handle;

        drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
      };

      auto error_gen = [&]() {
        std::memset(&dreq, 0, sizeof(dreq));

        dreq.handle = buf.handle;

        drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
      };

      std::memset(&creq, 0, sizeof(creq));

      creq.width = buf.width;
      creq.height = buf.height;
      creq.bpp = 32;

      if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0) {
        std::cout << "cannot create dumb buffer" << std::endl;

        return false;
      }

      buf.stride = creq.pitch;
      buf.size = creq.size;
      buf.handle = creq.handle;

      if(drmModeAddFB(
            fd,
            buf.width,
            buf.height,
            24,
            32,
            buf.stride,
            buf.handle,
            &buf.frame_buffer)) {
        std::cout << "cannot create framebuffer" << std::endl;

        error_gen();

        return false;
      }

      std::memset(&mreq, 0, sizeof(mreq));

      mreq.handle = buf.handle;

      if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq)) {
        std::cout << "cannot map dumb buffer" << std::endl;

        error_fb();

        return false;
      }

      buf.map = static_cast<uint8_t *>(mmap(
            0,
            buf.size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            mreq.offset));

      if (buf.map == MAP_FAILED) {
        std::cout << "cannot mmap dumb buffer" << std::endl;

        error_fb();

        return false;
      }

      return true;
    }

    static bool find_crtc(
        int32_t fd,
        drmModeRes * res, 
        drmModeConnector * con,
        std::vector<device> & devices,
        device & dev) {
      drmModeEncoder * enc;

      int32_t crtc;

      if (con->encoder_id)
        enc = drmModeGetEncoder(fd, con->encoder_id);

      if (enc) {
        if (enc->crtc_id) {
          crtc = enc->crtc_id;

          for (auto & de : devices)
            if (de.crtc == crtc) {
              crtc = -1;

              break;
            }

          if (crtc >= 0) {
            drmModeFreeEncoder(enc);

            dev.crtc = crtc;

            return true;
          }
        }

        drmModeFreeEncoder(enc);
      }

      for (int32_t i = 0; i < con->count_encoders; i++) {
        enc = drmModeGetEncoder(fd, con->encoders[i]);

        if (!enc) {
          std::cout << "cannot retrieve encoder " << i << " " <<
            con->encoders[i] << std::endl;

          continue;
        }

        for (int32_t j = 0; j < res->count_crtcs; j++) {
          if (!(enc->possible_crtcs & (1 << j)))
            continue;

          crtc = res->crtcs[j];

          for (auto & de : devices)
            if (de.crtc == crtc) {
              crtc = -1;

              break;
            }

          if (crtc >= 0) {
            drmModeFreeEncoder(enc);

            dev.crtc = crtc;

            return true;
          }
        }

        drmModeFreeEncoder(enc);
      }

      return false;
    }

    static bool setup_device(
        int32_t fd,
        drmModeRes * res,
        drmModeConnector * con,
        std::vector<device> & devices,
        device & dev) {
      if (con->connection != DRM_MODE_CONNECTED) {
        std::cout << "ignoring unused connector " <<
          con->connector_id << std::endl;

        return false;
      }

      if (con->count_modes == 0) {
        std::cout << "no valid mode for connector " <<
          con->connector_id << std::endl;

        return false;
      }

      std::memcpy(&dev.mode, &con->modes[0], sizeof(dev.mode));

      dev.buffers[0].width = con->modes[0].hdisplay;
      dev.buffers[0].height = con->modes[0].vdisplay;

      dev.buffers[1].width = con->modes[0].hdisplay;
      dev.buffers[1].height = con->modes[0].vdisplay;

      std::cout << "mode for connector " << con->connector_id << " is " <<
        dev.buffers[0].width << "x" << dev.buffers[0].height << std::endl;

      if (!find_crtc(fd, res, con, devices, dev)) {
        std::cout << "no valid crtc for connector " <<
          con->connector_id << std::endl;

        return false;
      }

      if (!create_fb(fd, dev.buffers[0])) {
        std::cout << "cannot create framebuffer for connector " <<
          con->connector_id << std::endl;

        return false;
      }

      if (!create_fb(fd, dev.buffers[1])) {
        std::cout << "cannot create framebuffer for connector " <<
          con->connector_id << std::endl;

        destroy_fb(fd, dev.buffers[0]);

        return false;
      }

      return true;
    }

    static bool open_devices(int32_t fd, std::vector<device> & devices) { 
      drmModeRes * res;
      drmModeConnector * con;

      res = drmModeGetResources(fd);

      if (!res) {
        std::cout << "cannot retrieve drm resources" << std::endl;

        return false;
      }

      for (uint32_t i = 0; i < res->count_connectors; i++) {
        con = drmModeGetConnector(fd, res->connectors[i]);

        if (!con) {
          std::cout << "cannot retrieve drm connector " <<
            res->connectors[i] << std::endl;

          continue;
        }

        device dev;

        dev.connector = con->connector_id;

        if (!setup_device(fd, res, con, devices, dev)) {
          std::cout << "cannot setup device for connector " <<
            res->connectors[i] << std::endl;

          drmModeFreeConnector(con);

          continue;
        }

        drmModeFreeConnector(con);

        devices.push_back(dev);
      }

      drmModeFreeResources(res);

      return true;
    }

    static bool init_devices(
        int32_t fd,
        std::vector<device> & devices) {
      for (auto & de : devices) {
        de.saved_crtc = drmModeGetCrtc(fd, de.crtc);

        if (drmModeSetCrtc(
              fd,
              de.crtc,
              (&de.buffers[de.front_buffer])->frame_buffer,
              0,
              0,
              &de.connector,
              1,
              &de.mode)) {
          std::cout << "cannot set crtc for connector " <<
            de.connector << std::endl;

          return false;
        }
      }

      return true;
    }

    static void close_devices(int32_t fd, std::vector<device> & devices) { 
      drmEventContext ev;

      std::memset(&ev, 0, sizeof(ev));

      ev.version = DRM_EVENT_CONTEXT_VERSION;
      ev.page_flip_handler = event::flip_event;

      for (auto & de : devices) {
        de.cleanup = true;

        std::cout << "wait for pending page flip to complete" << std::endl;

        while (de.page_flip_pending)
          if (drmHandleEvent(fd, &ev))
            break;

        if (!de.page_flip_pending)
          drmModeSetCrtc(
              fd,
              de.saved_crtc->crtc_id,
              de.saved_crtc->buffer_id,
              de.saved_crtc->x,
              de.saved_crtc->y,
              &de.connector,
              1,
              &de.saved_crtc->mode);

        drmModeFreeCrtc(de.saved_crtc);

        destroy_fb(fd, de.buffers[1]);
        destroy_fb(fd, de.buffers[0]);
      }
    }

  }

}

#endif
