## Process this file with automake to produce Makefile.in
#
#


DEPS = $(top_builddir)/src/libturboxsl.la

LDADDS = $(top_builddir)/src/libturboxsl.la

lib_LTLIBRARIES = libturboxsl.la

libturboxsl_la_SOURCES = \
  zlog/src/buf.c \
  zlog/src/buf.h \
  zlog/src/category.c \
  zlog/src/category.h \
  zlog/src/category_table.c \
  zlog/src/category_table.h \
  zlog/src/conf.c \
  zlog/src/conf.h \
  zlog/src/event.c \
  zlog/src/event.h \
  zlog/src/fmacros.h \
  zlog/src/format.c \
  zlog/src/format.h \
  zlog/src/level.c \
  zlog/src/level.h \
  zlog/src/level_list.c \
  zlog/src/level_list.h \
  zlog/src/mdc.c \
  zlog/src/mdc.h \
  zlog/src/record.c \
  zlog/src/record.h \
  zlog/src/record_table.c \
  zlog/src/record_table.h \
  zlog/src/rotater.c \
  zlog/src/rotater.h \
  zlog/src/rule.c \
  zlog/src/rule.h \
  zlog/src/spec.c \
  zlog/src/spec.h \
  zlog/src/thread.c \
  zlog/src/thread.h \
  zlog/src/version.h \
  zlog/src/zc_arraylist.c \
  zlog/src/zc_arraylist.h \
  zlog/src/zc_defs.h \
  zlog/src/zc_hashtable.c \
  zlog/src/zc_hashtable.h \
  zlog/src/zc_profile.c \
  zlog/src/zc_profile.h \
  zlog/src/zc_util.c \
  zlog/src/zc_util.h \
  zlog/src/zc_xplatform.h \
  zlog/src/zlog-chk-conf.c \
  zlog/src/zlog.c \
  zlog/src/zlog.h \
  logger.c \
  logger.h \
  thread_lock.c \
  thread_lock.h \
  concurrent_dictionary.c \
  concurrent_dictionary.h \
  unbounded_queue.c \
  unbounded_queue.h \
  shared_variable.c \
  shared_variable.h \
  template_task.c \
  template_task.h \
  template_task_graph.c \
  template_task_graph.h \
  threadpool.c \
  threadpool.h \
  localization.c \
  localization.h \
  parse.c \
  output.c \
  templates.c \
  templates.h \
  instructions.c \
  instructions.h \
  transform.c \
  xpath.c \
  variables.c \
  functions.c \
  nodes.c \
  utils.c \
  strings.c \
  strings.h \
  xmldict.c \
  md5.c \
  md5.h \
  md5_loc.h \
  rvalue.c \
  allocator.c \
  external_cache.c \
  external_cache.h \
  xmldict.h \
  allocator.h \
  xsl_elements.c \
  xsl_elements.h \
  group_rights.c \
  turboxsl.h \
  ltr_xsl.h

libturboxsl_la_LIBADD = -lpthread -lm -lmemcached -lck -lgettextpo

libturboxslincdir=$(includedir)/turboxsl
libturboxslinc_HEADERS = turboxsl.h

