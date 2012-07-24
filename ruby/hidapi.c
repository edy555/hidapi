#include "ruby.h"
#include "st.h"

#include "hidapi.h"
#include <stdlib.h>
#include <errno.h>

#ifndef RSTRING_PTR
# define RSTRING_PTR(s) (RSTRING(s)->ptr)
# define RSTRING_LEN(s) (RSTRING(s)->len)
#endif

#define MAX_STR 255


static VALUE rb_cHID;

static VALUE rhid_device_new(hid_device *h);

#if 0

#define define_hid_struct(c_name, ruby_name) \
  static VALUE rb_cHID_ ## ruby_name; \
  static st_table *c_name ## _objects; \
  typedef struct { struct hid_ ## c_name *ptr; VALUE parent; } rhid_ ## c_name ## _t; \
  static void rhid_ ## c_name ## _free(void *p) { \
    if (p) free(p); \
  } \
  static VALUE rhid_ ## c_name ## _make(struct hid_ ## c_name *p, VALUE parent) \
  { \
    VALUE v; \
    rhid_ ## c_name ## _t *d; \
    if (p == NULL) { return Qnil; } \
    if (st_lookup(c_name ## _objects, (st_data_t)p, (st_data_t *)&v)) \
      return v; \
    d = (rhid_ ## c_name ## _t *)xmalloc(sizeof(*d)); \
    d->ptr = p; \
    d->parent = parent; \
    v = Data_Wrap_Struct(rb_cHID_ ## ruby_name, 0, rhid_ ## c_name ## _free, d); \
    st_add_direct(c_name ## _objects, (st_data_t)p, (st_data_t)v); \
    return v; \
  } \
  static rhid_ ## c_name ## _t *check_hid_ ## c_name(VALUE v) \
  { \
    Check_Type(v, T_DATA); \
    if (RDATA(v)->dfree != rhid_ ## c_name ## _free) { \
      rb_raise(rb_eTypeError, "wrong argument type %s (expected HID::" #ruby_name ")", \
               rb_class2name(CLASS_OF(v))); \
    } \
    return DATA_PTR(v); \
  } \
  static rhid_ ## c_name ## _t *get_rhid_ ## c_name(VALUE v) \
  { \
    rhid_ ## c_name ## _t *d = check_hid_ ## c_name(v); \
    if (!d) { \
      rb_raise(rb_eArgError, "revoked HID::" #ruby_name); \
    } \
    return d; \
  } \
  static struct hid_ ## c_name *get_hid_ ## c_name(VALUE v) \
  { \
    return get_rhid_ ## c_name(v)->ptr; \
  } \
  static VALUE get_hid_ ## c_name ## _parent(VALUE v) \
  { \
    return get_rhid_ ## c_name(v)->parent; \
  }

define_hid_struct(device, Device)

static int mark_data_i(st_data_t key, st_data_t val, st_data_t arg)
{
  if (DATA_PTR((VALUE)val))
    rb_gc_mark((VALUE)val);
  return ST_CONTINUE;
}

VALUE rhid_gc_root;
static void rhid_gc_mark(void *p) {
  st_foreach(device_objects, mark_data_i, 0);
}

static int revoke_data_i(st_data_t key, st_data_t val, st_data_t arg)
{
  DATA_PTR((VALUE)val) = NULL;
  return ST_DELETE;
}

#endif

/* HID#enumerate */
static VALUE
//rhid_enumerate(VALUE cHID, VALUE vendor_id, VALUE product_id)
rhid_enumerate(int argc, VALUE *argv)
{
  VALUE vendor_id = Qnil;
  VALUE product_id = Qnil;
  int vid = 0;
  int pid = 0;
  int n;

  n = rb_scan_args(argc, argv, "02", &vendor_id, &product_id);
  if (n > 3) {
    rb_raise(rb_eArgError,"wrong number of arguments");
  }
  if (vendor_id != Qnil)
    vid = NUM2INT(vendor_id);
  if (product_id != Qnil)
    pid = NUM2INT(product_id);

  struct hid_device_info *devs;
  
  devs = hid_enumerate(vid, pid);
  if (devs == NULL) {
    rb_sys_fail("hid_enumerate");
  }

  char mbs[MAX_STR];
  VALUE ary = rb_ary_new();
  struct hid_device_info *di;
  for (di = devs; di != NULL; di = di->next) {
    VALUE h = rb_hash_new();
    rb_hash_aset(h, ID2SYM(rb_intern("path")), rb_str_new2(di->path));
    rb_hash_aset(h, ID2SYM(rb_intern("vendor_id")), INT2NUM(di->vendor_id));
    rb_hash_aset(h, ID2SYM(rb_intern("product_id")), INT2NUM(di->product_id));
    wcstombs(mbs, di->serial_number, MAX_STR);
    rb_hash_aset(h, ID2SYM(rb_intern("serial_number")), rb_str_new2(mbs));
    rb_hash_aset(h, ID2SYM(rb_intern("release_number")), INT2NUM(di->release_number));
    wcstombs(mbs, di->manufacturer_string, MAX_STR);
    rb_hash_aset(h, ID2SYM(rb_intern("manufacturer_string")), rb_str_new2(mbs));
    wcstombs(mbs, di->product_string, MAX_STR);
    rb_hash_aset(h, ID2SYM(rb_intern("product_string")), rb_str_new2(mbs));
    rb_hash_aset(h, ID2SYM(rb_intern("usage_page")), INT2NUM(di->usage_page));
    rb_hash_aset(h, ID2SYM(rb_intern("usage")), INT2NUM(di->usage));
    rb_hash_aset(h, ID2SYM(rb_intern("interface_number")), INT2NUM(di->interface_number));
    rb_ary_push(ary, h);
  }
  hid_free_enumeration(devs);
  return ary;
}

static VALUE rb_cHID_Device;

void rhid_device_free(void *_h)
{
  hid_device *h = (hid_device *)_h;
  if (h) hid_close(h);
}

static VALUE
rhid_device_new(hid_device *h)
{
  return Data_Wrap_Struct(rb_cHID_Device, 0, rhid_device_free, h);
}

static hid_device *check_hid_device(VALUE v)
{
  Check_Type(v, T_DATA);
  if (RDATA(v)->dfree != rhid_device_free) {
    rb_raise(rb_eTypeError, "wrong argument type %s (expected HID::Device)",
             rb_class2name(CLASS_OF(v)));
  }
  return DATA_PTR(v);
}

static hid_device *get_hid_device(VALUE v)
{
  hid_device *p = check_hid_device(v);
  if (!p) {
    rb_raise(rb_eArgError, "closed HID::Device");
  }
  return p;
} 

static int check_hid_error(char *reason, int ret)
{
  if (ret < 0) {
    errno = -ret;
    rb_sys_fail(reason);
  }
  return ret;
}

/* HID::Device#hid_close */
static VALUE
rhid_device_close(VALUE v)
{
  hid_device *p = get_hid_device(v);
  hid_close(p);
  DATA_PTR(v) = NULL;
  return Qnil;
}

/* HID#open */
static VALUE
//rhid_open(VALUE cHID, VALUE vendor_id, VALUE product_id, VALUE vserial_number)
rhid_open(int argc, VALUE *argv)
{
  int vid;
  int pid;
  wchar_t *serial_number = NULL;
  hid_device *h;
  VALUE vendor_id, product_id;
  VALUE vserial_number = Qnil;
  int n;

  n = rb_scan_args(argc, argv, "21", &vendor_id, &product_id, &vserial_number);
  if (n < 2 || n > 3) {
    rb_raise(rb_eArgError,"wrong number of arguments");
  }
  vid = NUM2INT(vendor_id);
  pid = NUM2INT(product_id);

  if (n == 3) {
    size_t len;
    wchar_t *ws;
    StringValue(vserial_number);
    len = RSTRING_LEN(vserial_number);
    ws = alloca(sizeof(wchar_t) * (len + 1));
    mbstowcs(ws, RSTRING_PTR(vserial_number), len);
    serial_number = ws;
  }

  h = hid_open(vid, pid, serial_number);
  if (h == NULL) {
    rb_sys_fail("hid_open");
  }

  return rhid_device_new(h);
}

/* HID#open_path */
static VALUE
rhid_open_path(VALUE cHID, VALUE vpath)
{
  char *path;
  hid_device *h;
  
  StringValue(vpath);
  path = RSTRING_PTR(vpath);
  h = hid_open_path(path);
  if (h == NULL) {
    rb_sys_fail("hid_open_path");
  }

  return rhid_device_new(h);
}

/* HID::Device#write */
static VALUE
rhid_device_write(VALUE v, VALUE vdata)
{
  hid_device *device = get_hid_device(v);
  char *data;
  int length;
  int ret;
  StringValue(vdata);
  data = RSTRING_PTR(vdata);
  length = RSTRING_LEN(vdata);
  ret = hid_write(device, data, length);
  check_hid_error("hid_write", ret);
  return INT2NUM(ret);
}

/* HID::Device#read */
static VALUE
rhid_device_read(VALUE v, VALUE vdata)
{
  hid_device *device = get_hid_device(v);
  char *data;
  int length;
  int ret;
  StringValue(vdata);
  rb_str_modify(vdata);
  data = RSTRING_PTR(vdata);
  length = RSTRING_LEN(vdata);
  ret = hid_read(device, data, length);
  check_hid_error("hid_read", ret);
  return INT2NUM(ret);
}

/* HID::Device#set_nonblocking */
static VALUE
rhid_device_set_nonblocking(VALUE v, VALUE vnonblock)
{
  hid_device *device = get_hid_device(v);
  int nonblock = NUM2INT(vnonblock);
  int ret;

  ret = hid_set_nonblocking(device, nonblock);
  check_hid_error("hid_set_nonblocking", ret);
  return INT2NUM(ret);
}


/* HID::Device#send_feature_report */
static VALUE
rhid_device_send_feature_report(VALUE v, VALUE vdata)
{
  hid_device *device = get_hid_device(v);
  char *data;
  int length;
  int ret;
  StringValue(vdata);
  data = RSTRING_PTR(vdata);
  length = RSTRING_LEN(vdata);
  ret = hid_send_feature_report(device, data, length);
  check_hid_error("hid_send_feature_report", ret);
  return INT2NUM(ret);
}

/* HID::Device#get_feature_report */
static VALUE
rhid_device_get_feature_report(VALUE v, VALUE vdata)
{
  hid_device *device = get_hid_device(v);
  char *data;
  int length;
  int ret;
  StringValue(vdata);
  rb_str_modify(vdata);
  data = RSTRING_PTR(vdata);
  length = RSTRING_LEN(vdata);
  ret = hid_get_feature_report(device, data, length);
  check_hid_error("hid_get_feature_report", ret);
  return INT2NUM(ret);
}



/* HID::Device#get_manufacturer_string */
static VALUE
rhid_device_get_manufacturer_string(VALUE v)
{
  hid_device *device = get_hid_device(v);
  wchar_t wcs[MAX_STR];
  char mbs[MAX_STR];
  int ret = hid_get_manufacturer_string(device, wcs, MAX_STR);
  check_hid_error("hid_get_manufacturer_string", ret);
  wcstombs(mbs, wcs, MAX_STR);
  return rb_str_new2(mbs);
}

/* HID::Device#get_product_string */
static VALUE
rhid_device_get_product_string(VALUE v)
{
  hid_device *device = get_hid_device(v);
  wchar_t wcs[MAX_STR];
  char mbs[MAX_STR];
  int ret = hid_get_product_string(device, wcs, MAX_STR);
  check_hid_error("hid_get_product_string", ret);
  wcstombs(mbs, wcs, MAX_STR);
  return rb_str_new2(mbs);
}

/* HID::Device#get_serial_number_string */
static VALUE
rhid_device_get_serial_number_string(VALUE v)
{
  hid_device *device = get_hid_device(v);
  wchar_t wcs[MAX_STR];
  char mbs[MAX_STR];
  int ret = hid_get_serial_number_string(device, wcs, MAX_STR);
  check_hid_error("hid_get_serial_number_string", ret);
  wcstombs(mbs, wcs, MAX_STR);
  return rb_str_new2(mbs);
}

/* HID::Device#get_indexed_string */
static VALUE
rhid_device_get_indexed_string(VALUE v, VALUE vindex)
{
  hid_device *device = get_hid_device(v);
  int index = NUM2INT(vindex);
  wchar_t wcs[MAX_STR];
  char mbs[MAX_STR];
  int ret = hid_get_indexed_string(device, index, wcs, MAX_STR);
  check_hid_error("hid_get_indexed_string", ret);
  wcstombs(mbs, wcs, MAX_STR);
  return rb_str_new2(mbs);
}


void
Init_hidapi()
{
  rb_cHID = rb_define_module("HID");
  rb_define_module_function(rb_cHID, "enumerate", rhid_enumerate, -1);
  rb_define_module_function(rb_cHID, "open", rhid_open, -1);
  rb_define_module_function(rb_cHID, "open_path", rhid_open_path, 1);

  rb_cHID_Device = rb_define_class_under(rb_cHID, "Device", rb_cData);
  rb_define_method(rb_cHID_Device, "write", rhid_device_write, 1);
  rb_define_method(rb_cHID_Device, "read", rhid_device_read, 1);
  rb_define_method(rb_cHID_Device, "set_nonblocking", rhid_device_set_nonblocking, 1);
  rb_define_method(rb_cHID_Device, "close", rhid_device_close, 0);

  rb_define_method(rb_cHID_Device, "send_feature_report", rhid_device_send_feature_report, 1);
  rb_define_method(rb_cHID_Device, "get_feature_report", rhid_device_get_feature_report, 1);

  rb_define_method(rb_cHID_Device, "get_manufacturer_string", rhid_device_get_manufacturer_string, 0);
  rb_define_method(rb_cHID_Device, "get_product_string", rhid_device_get_product_string, 0);
  rb_define_method(rb_cHID_Device, "get_serial_number_string", rhid_device_get_serial_number_string, 0);
  rb_define_method(rb_cHID_Device, "get_indexed_string", rhid_device_get_indexed_string, 1);
  //rb_define_method(rb_cHID_Device, "error", rhid_device_error, 0);
}
