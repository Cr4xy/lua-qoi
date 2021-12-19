/*
 * Lua Bindings to the QOI library
 * 2021 Cr4xy <cr4xy@live.de> Initial version
 *
 * SPDX-License-Identifier: MIT
 */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "lua-compat-5.3/c-api/compat-5.3.h"

#define QOI_IMPLEMENTATION
#include "qoi/qoi.h"

#define boxptr(L, p)   (*(void**)(lua_newuserdata(L, sizeof(void*))) = (p))
#define unboxptr(L, i) (*(void**)(lua_touserdata(L, i)))

#define LUA_QOI_IMAGE_PTR_TYPENAME "qoiImagePtr_handle"

struct qoiImgData {
    qoi_desc desc;
    void *data;
};

typedef struct qoiImgData* qoiImgDataPtr;

static int typerror(lua_State *L, int narg, const char *tname) {
    const char *msg = lua_pushfstring(L, "%s expected, got %s", tname, luaL_typename(L, narg));
    return luaL_argerror(L, narg, msg);
}

static qoiImgDataPtr getImagePtr(lua_State *L, int i) {
    int pop = 0;
    if (lua_istable(L, i)) {
        lua_getfield(L, i, "__ptr");
        pop = 1;
    }
    void *ud = luaL_checkudata(L, -1, LUA_QOI_IMAGE_PTR_TYPENAME);
    if (ud != NULL) {
        qoiImgDataPtr im = unboxptr(L, -1);
        if (im == NULL) {
            lua_pop(L, pop);
            luaL_error(L, "attempt to use an invalid " LUA_QOI_IMAGE_PTR_TYPENAME);
            return NULL;
        }
        lua_pop(L, pop);
        return im;
    }
    typerror(L, i, LUA_QOI_IMAGE_PTR_TYPENAME);
    lua_pop(L, pop);
    return NULL;
}

static int LqoiImageSetPixel(lua_State *L);
static int LqoiImageGetPixel(lua_State *L);
static int LqoiImageWrite(lua_State *L);
static int LqoiImageEncode(lua_State *L);

static void pushImagePtr(lua_State *L, qoiImgDataPtr im) {
    lua_newtable(L);
    lua_pushstring(L, "__ptr");
    boxptr(L, im);
    luaL_getmetatable(L, LUA_QOI_IMAGE_PTR_TYPENAME);
    lua_setmetatable(L, -2);
    lua_settable(L, -3);
    lua_pushstring(L, "width");
    lua_pushnumber(L, im->desc.width);
    lua_settable(L, -3);
    lua_pushstring(L, "height");
    lua_pushnumber(L, im->desc.height);
    lua_settable(L, -3);
    lua_pushstring(L, "channels");
    lua_pushnumber(L, im->desc.channels);
    lua_settable(L, -3);

    lua_pushstring(L, "setPixel");
    lua_pushcfunction(L, LqoiImageSetPixel);
    lua_settable(L, -3);
    lua_pushstring(L, "getPixel");
    lua_pushcfunction(L, LqoiImageGetPixel);
    lua_settable(L, -3);
    lua_pushstring(L, "write");
    lua_pushcfunction(L, LqoiImageWrite);
    lua_settable(L, -3);
    lua_pushstring(L, "encode");
    lua_pushcfunction(L, LqoiImageEncode);
    lua_settable(L, -3);

    luaL_getmetatable(L, LUA_QOI_IMAGE_PTR_TYPENAME);
    lua_setmetatable(L, -2);
}

static int LqoiRead(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    int channels = 0;
    if (lua_isnumber(L, 2)) {
        channels = lua_tonumber(L, 2);
    }
    qoi_desc desc;
    qoiImgDataPtr img = (qoiImgDataPtr)lua_newuserdata(L, sizeof(struct qoiImgData));
    void *rgba_pixels = qoi_read(path, &desc, channels);
    if (rgba_pixels == NULL) {
        lua_pushnil(L);
        return 1;
    }
    img->desc = desc;
    img->data = rgba_pixels;
    pushImagePtr(L, img);
    return 1;
}

static int LqoiImageWrite(lua_State *L) {
    qoiImgDataPtr img = getImagePtr(L, 1);
    const char *path = luaL_checkstring(L, 2);
    if (img == NULL) {
        lua_pushnil(L);
        return 1;
    }
    if (qoi_write(path, img->data, &img->desc) != 0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

static int LqoiImageEncode(lua_State *L) {
    qoiImgDataPtr img = getImagePtr(L, 1);
    if (img == NULL) {
        lua_pushnil(L);
        return 1;
    }
    size_t len = 0;
    void* encoded = qoi_encode(img->data, &img->desc, &len);
    lua_pushlstring(L, encoded, len);
    return 1;
}

static int LqoiDecode(lua_State *L) {
    const char *data = luaL_checkstring(L, 1);
    //#if LUA_VERSION_NUM == 501
    //size_t len = lua_objlen(L, 1);
    //#else
    size_t len = lua_rawlen(L, 1);
    //#endif
    qoi_desc desc;
    qoiImgDataPtr img = (qoiImgDataPtr)lua_newuserdata(L, sizeof(struct qoiImgData));
    void *rgba_pixels = qoi_decode(data, len, &desc, 4);
    if (rgba_pixels == NULL) {
        lua_pushnil(L);
        return 1;
    }
    img->desc = desc;
    img->data = rgba_pixels;
    pushImagePtr(L, img);
    return 1;
}

static int LqoiNew(lua_State *L) {
    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);
    int channels = 4;
    if (lua_isnumber(L, 3)) {
        channels = lua_tonumber(L, 3);
    }
    qoi_desc desc = {
        .width = width,
        .height = height,
        .channels = channels,
        .colorspace = QOI_SRGB,
    };
    qoiImgDataPtr img = (qoiImgDataPtr)lua_newuserdata(L, sizeof(struct qoiImgData));
    void *rgba_pixels = malloc(width * height * channels * sizeof(unsigned char));
    memset(rgba_pixels, 0, width * height * channels * sizeof(unsigned char));
    if (rgba_pixels == NULL) {
        lua_pushnil(L);
        return 1;
    }
    img->desc = desc;
    img->data = rgba_pixels;
    pushImagePtr(L, img);
    return 1;
}

static int LqoiImageDestroy(lua_State *L) {
    qoiImgDataPtr img = getImagePtr(L, 1);
    if (img == NULL || img->data == NULL) {
        return 0;
    }
    free(img->data);
    img->data = NULL;
    return 0;
}

static int LqoiImageToString(lua_State *L) {
    qoiImgDataPtr img = getImagePtr(L, 1);
    if (img == NULL) {
        return 0;
    }
    lua_pushfstring(L, "qoiImage: %p (%dx%d)", img, img->desc.width, img->desc.height);
    return 1;
}

static int LqoiImageGetPixel(lua_State *L) {
    qoiImgDataPtr img = getImagePtr(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    if (img == NULL) {
        return 0;
    }
    unsigned char* rgba_pixels = (unsigned char*)img->data;

    for (int i = 0; i < img->desc.channels; i++) {
        lua_pushnumber(L, rgba_pixels[(y * img->desc.width + x) * img->desc.channels + i]);
    }
    return img->desc.channels;
}

static int LqoiImageSetPixel(lua_State *L) {
    qoiImgDataPtr img = getImagePtr(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int r = luaL_checkinteger(L, 4);
    int g = luaL_checkinteger(L, 5);
    int b = luaL_checkinteger(L, 6);
    int a = 255;
    if (lua_isnumber(L, 7)) {
        a = lua_tonumber(L, 7);
    }
    if (img == NULL) {
        return 0;
    }
    unsigned char* rgba_pixels = (unsigned char*)img->data;
    for (int i = 0; i < img->desc.channels; i++) {
        rgba_pixels[(y * img->desc.width + x) * img->desc.channels + i] = (i == 0) ? r : (i == 1) ? g : (i == 2) ? b : a;
    }
    return 0;
}

static const luaL_Reg LqoiLib[] = {
    {"new", LqoiNew},
    {"read", LqoiRead},
    {"decode", LqoiDecode},
    {"encode", LqoiImageEncode},
    {"write", LqoiImageWrite},
    {"destroy", LqoiImageDestroy},
    {"getPixel", LqoiImageGetPixel},
    {"setPixel", LqoiImageSetPixel},
    { NULL, NULL }
};

#ifndef QOI_API
#define QOI_API
#endif

QOI_API __declspec(dllexport) int luaopen_qoi(lua_State *L) {
    luaL_newmetatable(L, LUA_QOI_IMAGE_PTR_TYPENAME);
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    lua_pushliteral(L, "__tostring");
    lua_pushcfunction(L, LqoiImageToString);
    lua_settable(L, -3);
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, LqoiImageDestroy);
    lua_settable(L, -3);

    luaL_newlib(L, LqoiLib);
    return 1;
}