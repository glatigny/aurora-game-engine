# Default target
all: lib

###########################################################
# Sources

SRC += audio_event.cpp
SRC += graphical_effects.cpp
SRC += renderable.cpp
SRC += vfs_file.cpp
SRC += script_monitor.cpp
SRC += edit_line.cpp
SRC += aoe_object.cpp
SRC += realtime.cpp
SRC += entity.cpp
SRC += rect.cpp
SRC += renderable_interpolator.cpp
SRC += collidable.cpp
SRC += audible.cpp
SRC += sound_engine.cpp
SRC += network_engine.cpp
SRC += pspec/default_time_engine.cpp
SRC += playback_track.cpp
SRC += pspec/utils.cpp
#SRC += savable.cpp
SRC += factory.cpp
SRC += object_mold.cpp
SRC += interpolation.cpp
SRC += text_line.cpp
SRC += text_box.cpp
SRC += font.cpp
SRC += glyph.cpp
SRC += bitmap_font.cpp
SRC += bitmap_glyph.cpp
SRC += context.cpp
SRC += context_initializer.cpp
SRC += aurora_engine.cpp
SRC += hardware_engine.cpp
SRC += graphical_engine.cpp
SRC += input_engine.cpp
SRC += scriptable_object.cpp
SRC += script_variable.cpp
SRC += scripted_function.cpp
SRC += script_map.cpp
SRC += state.cpp
SRC += utils.cpp
SRC += controller_state.cpp
SRC += point.cpp
SRC += frame.cpp
SRC += collision_hull.cpp
SRC += collision_point.cpp
SRC += collision_rect.cpp
SRC += decoration.cpp
SRC += game_object.cpp
SRC += game_map.cpp
SRC += sprite.cpp
SRC += stance.cpp
SRC += animation.cpp
SRC += key_animation.cpp
SRC += script_trigger.cpp
SRC += section_container.cpp
SRC += text.cpp
SRC += sentence.cpp
SRC += menu.cpp
SRC += sub_scriptable.cpp
SRC += signalable.cpp
SRC += menu_item.cpp
SRC += animated_menu.cpp
SRC += animated_menu_item.cpp
#SRC += renderable_menu.cpp
#SRC += renderable_menu_item.cpp
SRC += effect_interpolator.cpp
SRC += effect_key.cpp
SRC += flying_particle.cpp
SRC += scriptable_particle.cpp
SRC += cameraman.cpp
SRC += scriptable_input.cpp
SRC += vfs/file.cpp
SRC += vfs/image_source.cpp
SRC += vfs/buffer_image_source.cpp

SRC_SCRIPT_DEBUGGER += script_debugger.cpp

SRC_DIRECTX += pspec/directx/directx.cpp
SRC_DIRECTX += pspec/directx/win32_init_window.cpp

SRC_SDL += pspec/sdl_gl/sdl_gl.cpp
SRC_SDL += pspec/sdl_gl/sdl_init.cpp
SRC_SDL += pspec/sdl_gl/sdl_input.cpp
SRC_OPENGL += pspec/open_gl/open_gl.cpp
SRC_OPENGL += pspec/open_gl/gl_framebuffer.cpp
WIN32_SRC_OPENGL += pspec/open_gl/wgl_init.cpp

WIN32_SRC += pspec/win32/time.cpp

WIN32_SRC_LOG_WINDOW += pspec/win32/win32syslog.cpp

SRC_OPENAL += pspec/open_al/open_al.cpp

SRC_POSIX += vfs/posix_file.cpp

SRC_PCM += vfs/pcm_source.cpp

SRC_VORBIS += vfs/decoders/vorbis_pcm_source.cpp

SRC_THREADS += pspec/thread.cpp
SRC_THREADS += background_task_scheduler.cpp

SRC_PNG += vfs/decoders/png_image_source.cpp

SRC_XML += vfs/xml_source.cpp

SRC_TCP += pspec/tcp_net.cpp

# Supplementary objects, relative to the top-level makefile
SUPP_OBJ += tinyxml/tinystr$(OBJEXT)
SUPP_OBJ += tinyxml/tinyxml$(OBJEXT)
SUPP_OBJ += tinyxml/tinyxmlerror$(OBJEXT)
SUPP_OBJ += tinyxml/tinyxmlparser$(OBJEXT)

include mk/lib.mk
