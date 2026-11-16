/*
  Copyright (C) 2019-2020 XC5 Limited, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  http://www.xcalibyte.com

  For more information, see:
  http://github.com/open64-compiler/open64
  http://gitee.com/open64-compiler/open64

*/

#include "c2w_dst.h"
#include "c2w_builder.h"
#include "c2w_tracer.h"

// clang header files
#include "clanginc.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/AST/RecordLayout.h"

using namespace clang;

// open64 header files
#include "open64inc.h"
#include "dwarf.h"
#include "dwarf_DST.h"
#include "dwarf_DST_producer.h"
#include "c2w_version.h"

// stdc++
#include <string>
#include <vector>

namespace wgen {

WhirlDstBuilder::WhirlDstBuilder(WhirlBuilder *builder)
  : _builder(builder) {
}

WhirlDstBuilder::~WhirlDstBuilder() {
}

void
WhirlDstBuilder::Initialize() {
  TRACE_FUNC();
  DST_Init(NULL, 0);
}

void
WhirlDstBuilder::Finalize() {
  TRACE_FUNC();
}


DST_language
WhirlDstBuilder::GetLanguage() const {
  switch (_builder->Lang()) {
    case IL_C:
      return DW_LANG_C89;
    case IL_CPP:
      return DW_LANG_C_plus_plus;
    default:
      return DW_LANG_C89;
  }
}

DST_identifier_case
WhirlDstBuilder::GetIdentifierCase() const {
  return DW_ID_case_sensitive;
}

const char *
WhirlDstBuilder::GetProducer() const {
  return COMPILER_FULL_VERSION;
}

static const char *
DropDirectory(const char *file) {
  const char *ptr = file;
  char ch;
  while ((ch = *ptr) != '\0') {
    ++ ptr;
    if (ch == '\\' || ch == '/')
      file = ptr;
  }
  return file;
}

static void
SplitFile(const char *fname, std::string &file, std::string &dir) {
  const char *sav = fname;
  const char *ptr = fname;
  char ch;
  while ((ch = *ptr) != '\0') {
    ++ ptr;
    if (ch == '\\' || ch == '/')
      fname = ptr;
  }
  file.assign(fname);
  if (fname != sav)
    dir.assign(sav, fname - sav - 1);
}

bool
WhirlDstBuilder::GetFileInfo(const FileEntry *fe, std::string &file, std::string &dir,
                             off_t *psize, time_t *ptime) {
  Is_True(fe, ("FileEntry is NULL"));
  file = fe->getName().str();
  if (file.empty())
    file = "<stdin>";
  else
    file = DropDirectory(file.c_str());
  SourceManager &SM = _builder->Context()->getSourceManager();
  dir = SM.getFileManager().getCanonicalName(fe->getDir()).str();
  if (psize)
    *psize = fe->getSize();
  if (ptime)
    *ptime = fe->getModificationTime();
  return true;
}

UINT32
WhirlDstBuilder::WhirlDirID(const char *dir) {
  STR_IDX_MAP::iterator it = _dir_map.find(dir);
  if (it != _dir_map.end())
    return it->second;
  DST_mk_include_dir(const_cast<char *>(dir));
  UINT32 dir_num = _dir_map.size() + 1;
  _dir_map[dir] = dir_num;
  return dir_num;
}

UINT32
WhirlDstBuilder::WhirlFileID(const char* fname) {
  SourceManager &SM = _builder->Context()->getSourceManager();
  llvm::ErrorOr<const FileEntry *> ret = SM.getFileManager().getFile(fname);
  if (ret && ret.get()) {
    return WhirlFileID(ret.get());
  }
  STR_IDX_MAP::iterator it = _file_map.find(fname);
  if (it != _file_map.end())
    return it->second;
  std::string file;
  std::string dir;
  SplitFile(fname, file, dir);
  DST_mk_file_name(const_cast<char *>(file.c_str()),
                   WhirlDirID(dir.c_str()), 0, 0);
  UINT32 file_num = _file_map.size() + 1;
  _file_map[fname] = file_num;
  return file_num;
}

UINT32
WhirlDstBuilder::WhirlFileID(const FileID id) {
  SourceManager &SM = _builder->Context()->getSourceManager();
  const FileEntry *fe = SM.getFileEntryForID(id);
  Is_True(fe, ("bad FileEntry"));
  return WhirlFileID(fe);
}

UINT32
WhirlDstBuilder::WhirlFileID(const FileEntry *fe) {
  Is_True(fe, ("bad FileEntry"));
  const char* fname = fe->getName().data();
  STR_IDX_MAP::iterator it = _file_map.find(fname);
  if (it != _file_map.end())
    return it->second;
  std::string file;
  std::string dir;
  off_t file_size = 0;
  time_t mod_time = 0;
  GetFileInfo(fe, file, dir, &file_size, &mod_time);
  DST_mk_file_name(const_cast<char *>(file.c_str()),
                   WhirlDirID(dir.c_str()), file_size, mod_time);
  UINT32 file_num = _file_map.size() + 1;
  _file_map[fname] = file_num;
  return file_num;
}

USRCPOS
WhirlDstBuilder::GetSrcPos(SourceLocation sl) {
  if (sl.isInvalid())
    return _cur_loc;
  USRCPOS_clear(_cur_loc);
  PresumedLoc ploc = _builder->Context()->getSourceManager().getPresumedLoc(sl);
  // Newer CLANG has PresumedLoc::getFileID(), use code below:
  // USRCPOS_filenum(_cur_loc) = WhirlFileID(ploc.getFileID());
  USRCPOS_filenum(_cur_loc) = WhirlFileID(ploc.getFilename());
  USRCPOS_linenum(_cur_loc) = ploc.getLine();
  USRCPOS_column(_cur_loc) = ploc.getColumn();
  return _cur_loc;
}

SRCPOS
WhirlDstBuilder::SetSrcPos(SourceLocation sl) {
  return USRCPOS_srcpos(GetSrcPos(sl));
}

SRCPOS
WhirlDstBuilder::GetSrcPos(void) {
  return USRCPOS_srcpos(_cur_loc);
}

  
DST_INFO_IDX
WhirlDstBuilder::CreateDstForCompileUnit() {
  TRACE_FUNC();
  SourceManager &SM = _builder->Context()->getSourceManager();
  std::string dir;
  std::string file;
  const FileEntry *fe = SM.getFileEntryForID(SM.getMainFileID());
  GetFileInfo(fe, file, dir, NULL, NULL);
  return DST_mk_compile_unit(const_cast<char *>(file.c_str()),
                             const_cast<char *>(dir.c_str()),
                             const_cast<char *>(GetProducer()),
                             GetLanguage(),
                             GetIdentifierCase());
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForStmt(const Stmt *stmt) {
  TRACE_FUNC();
  DST_INFO_IDX dst_idx = DST_mk_lexical_block(NULL, (void *) 0, (void *) 0, DST_INVALID_IDX);
  return dst_idx;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForFunc(ST_IDX st, TY_IDX rtype) {
  USRCPOS spos;
  USRCPOS_clear(spos);
  DST_INFO_IDX type_dst = DST_INVALID_IDX;
  DST_INFO_IDX func_dst = DST_mk_subprogram(spos,
                                            const_cast<char *>(ST_name(st)),
                                            type_dst,
                                            DST_INVALID_IDX,
                                            (void *) (INTPTR) st,
                                            DW_INL_not_inlined,
                                            DW_VIRTUALITY_none,
                                            0,                    // vtbl index
                                            false,                // is decl
                                            false,                // prototyped
                                            true,                 // artificial
                                            true);                // internal
  return func_dst;
}

// Variables, parameters and types are described from -g2 up, as wgen
// does (Debug_Level >= 2); -g1 gives functions and line numbers only.
static inline bool
WantFullDebugInfo() {
  return Debug_Level >= 2;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForFunctionDecl(GlobalDecl gd) {
  const FunctionDecl* decl = cast<FunctionDecl>(gd.getDecl());
  USRCPOS usrcpos = GetSrcPos(decl->getLocation());
  std::string name;
  if (decl->getIdentifier())
    name = decl->getName().str();
  else
    name = decl->getNameAsString();
  DST_INFO_IDX type_dst = DST_INVALID_IDX;
  if (WantFullDebugInfo())
    type_dst = CreateDstForType(decl->getReturnType());
  ST_IDX st_idx = _builder->SB().ConvertSymbol(gd);

  DST_INFO_IDX dst = DST_mk_subprogram(usrcpos,
                           const_cast<char *>(name.c_str()),
                           type_dst,
                           DST_INVALID_IDX,
                           (void *) (INTPTR) st_idx,
                           DW_INL_not_inlined, // TODO
                           DW_VIRTUALITY_none, // TODO
                           0,                  // vtbl index
                           false,              // is decl
                           decl->hasPrototype(),
                           false,              // artificial
                           !ST_is_export_local(ST_ptr(st_idx))); // external
  // As wgen: under the compile unit, and named by its mangled name too
  // when that differs (C++).
  DST_append_child(_builder->Scope().GlobalDst(), dst);
  const char *linkage_name = ST_name(st_idx);
  if (linkage_name && strcmp(linkage_name, name.c_str()) != 0 && !name.empty())
    DST_add_linkage_name_to_subprogram(dst, const_cast<char *>(linkage_name));
  return dst;
}

void
WhirlDstBuilder::CreateDstForParm(const ParmVarDecl *decl, ST_IDX st) {
  if (!WantFullDebugInfo())
    return;
  DST_INFO_IDX parent = _builder->Scope().CurrentDst();
  if (DST_IS_NULL(parent))
    return;
  std::string name = decl->getName().str();
  DST_INFO_IDX dst =
    DST_mk_formal_parameter(GetSrcPos(decl->getLocation()),
                            const_cast<char *>(name.c_str()),
                            CreateDstForType(decl->getType()),
                            (void *) (INTPTR) st,
                            DST_INVALID_IDX,  // abstract origin
                            DST_INVALID_IDX,  // default value
                            FALSE,            // optional
                            FALSE,            // variable
                            decl->isImplicit(),
                            FALSE);           // declaration only
  DST_append_child(parent, dst);
}

void
WhirlDstBuilder::CreateDstForVarargs(const FunctionDecl *decl) {
  if (!WantFullDebugInfo() || !decl->isVariadic())
    return;
  DST_INFO_IDX parent = _builder->Scope().CurrentDst();
  if (DST_IS_NULL(parent))
    return;
  DST_append_child(parent,
                   DST_mk_unspecified_parameters(GetSrcPos(decl->getLocation()),
                                                 DST_INVALID_IDX));
}

void
WhirlDstBuilder::CreateDstForVar(const VarDecl *decl, ST_IDX st) {
  if (!WantFullDebugInfo() || isa<ParmVarDecl>(decl))
    return;
  std::string name = decl->getName().str();
  if (name.empty())
    return;
  // Locals (including static ones) belong to their function, as with
  // wgen; everything else, block-scope extern declarations included,
  // to the compile unit.
  bool local = decl->isLocalVarDecl() && !decl->hasExternalStorage();
  DST_INFO_IDX parent = local ? _builder->Scope().CurrentDst()
                              : _builder->Scope().GlobalDst();
  if (DST_IS_NULL(parent))
    return;
  ST *s = ST_ptr(st);
  DST_INFO_IDX dst =
    DST_mk_variable(GetSrcPos(decl->getLocation()),
                    const_cast<char *>(name.c_str()),
                    CreateDstForType(decl->getType()),
                    0,                         // offset (Fortran)
                    (void *) (INTPTR) st,
                    DST_INVALID_IDX,           // abstract origin
                    ST_sclass(s) == SCLASS_EXTERN, // declaration
                    FALSE,                     // automatic (wgen: FALSE)
                    !ST_is_export_local(s),    // external
                    FALSE);                    // artificial
  DST_append_child(parent, dst);
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForVarDecl(const VarDecl *decl) {
  TRACE_FUNC();
  return DST_INVALID_IDX;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForDecl(const Decl *decl) {
  TRACE_FUNC();
  DST_INFO_IDX ret_dst;
  switch (decl->getKind()) {
    case Decl::Function:
    case Decl::CXXMethod:
    case Decl::CXXConversion:
      ret_dst = CreateDstForFunctionDecl(GlobalDecl(cast<FunctionDecl>(decl)));
      // (CreateDstForFunctionDecl puts it under the compile unit)
      return ret_dst;
    case Decl::CXXConstructor:
    case Decl::CXXDestructor:
      Is_True(FALSE, ("FIXME: ctor/dtor"));
      ret_dst = DST_INVALID_IDX;
      break;
    default:
      ret_dst = DST_INVALID_IDX;
      break;
  }
  if (!DST_IS_NULL(ret_dst)) {
    Is_True(!DST_IS_NULL(_builder->Scope().CurrentDst()), ("cur dst is NULL"));
    DST_append_child(_builder->Scope().CurrentDst(), ret_dst);
  }
  return ret_dst;
}

// Types, after wgen's Create_DST_type_For_Tree. All go under the compile
// unit; each Clang type (qualifiers included) gets one entry.

DST_INFO_IDX
WhirlDstBuilder::AppendType(DST_INFO_IDX dst) {
  if (!DST_IS_NULL(dst))
    DST_append_child(_builder->Scope().GlobalDst(), dst);
  return dst;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForBuiltinType(const BuiltinType *type) {
  TRACE_FUNC();
  ASTContext *ctx = _builder->Context();
  DST_size_t size = ctx->getTypeSize(type) / 8;
  if (type->getKind() == BuiltinType::Void)
    return DST_INVALID_IDX;
  if (type->getKind() == BuiltinType::NullPtr)
    return AppendType(DST_mk_pointer_type(DST_INVALID_IDX, DW_ADDR_none, size));
  DST_ATE_encoding enc;
  switch (type->getKind()) {
    case BuiltinType::Bool:
      enc = DW_ATE_boolean;
      break;
    case BuiltinType::Char_S:
    case BuiltinType::SChar:
      enc = DW_ATE_signed_char;
      break;
    case BuiltinType::Char_U:
    case BuiltinType::UChar:
      enc = DW_ATE_unsigned_char;
      break;
    default:
      if (type->isFloatingPoint())
        enc = DW_ATE_float;
      else if (type->isSignedInteger())
        enc = DW_ATE_signed;
      else if (type->isUnsignedInteger())
        enc = DW_ATE_unsigned;
      else
        return DST_INVALID_IDX;   // OpenCL, ObjC and other special types
      break;
  }
  std::string name = type->getName(ctx->getPrintingPolicy()).str();
  std::map<std::string, DST_INFO_IDX>::iterator it = _base_types.find(name);
  if (it != _base_types.end())
    return it->second;
  DST_INFO_IDX dst = AppendType(DST_mk_basetype(name.c_str(), enc, size));
  _base_types[name] = dst;
  return dst;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForFunctionType(const FunctionType *type) {
  TRACE_FUNC();
  const FunctionProtoType *proto = dyn_cast<FunctionProtoType>(type);
  USRCPOS src;
  USRCPOS_clear(src);
  DST_INFO_IDX dst = DST_mk_subroutine_type(src, "",
                                            CreateDstForType(type->getReturnType()),
                                            DST_INVALID_IDX,
                                            proto != NULL);
  AppendType(dst);
  if (proto) {
    for (unsigned i = 0; i < proto->getNumParams(); i++) {
      DST_INFO_IDX parm =
        DST_mk_formal_parameter(src, NULL,
                                CreateDstForType(proto->getParamType(i)),
                                NULL, DST_INVALID_IDX, DST_INVALID_IDX,
                                FALSE, FALSE, FALSE,
                                TRUE);   // declaration only
      DST_append_child(dst, parm);
    }
  }
  if (!proto || proto->isVariadic())
    DST_append_child(dst, DST_mk_unspecified_parameters(src, DST_INVALID_IDX));
  return dst;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForArrayType(const ArrayType *type) {
  TRACE_FUNC();
  // One array type with a subrange per dimension, as wgen does; a
  // dimension without a constant size (incomplete, VLA) ends the list.
  std::vector<INT64> dims;
  QualType elt(type, 0);
  bool known = true;
  while (const ArrayType *at = _builder->Context()->getAsArrayType(elt)) {
    if (const ConstantArrayType *cat = dyn_cast<ConstantArrayType>(at)) {
      if (known)
        dims.push_back(cat->getSize().getSExtValue());
    } else
      known = false;
    elt = at->getElementType();
  }
  USRCPOS src;
  USRCPOS_clear(src);
  DST_INFO_IDX dst = DST_mk_array_type(src, NULL, CreateDstForType(elt), 0,
                                       DST_INVALID_IDX, TRUE);
  AppendType(dst);
  for (size_t i = 0; i < dims.size(); i++) {
    DST_cval_ref lb, ub;
    lb.cval = 0;
    ub.cval = dims[i] - 1;
    DST_append_child(dst, DST_mk_subrange_type(TRUE, lb, TRUE, ub));
  }
  return dst;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForVectorType(const VectorType *type) {
  TRACE_FUNC();
  USRCPOS src;
  USRCPOS_clear(src);
  DST_INFO_IDX dst = DST_mk_array_type(src, NULL,
                                       CreateDstForType(type->getElementType()),
                                       0, DST_INVALID_IDX, TRUE);
  AppendType(dst);
  DST_cval_ref lb, ub;
  lb.cval = 0;
  ub.cval = type->getNumElements() - 1;
  DST_append_child(dst, DST_mk_subrange_type(TRUE, lb, TRUE, ub));
  DST_SET_GNU_vector(DST_INFO_flag(DST_INFO_IDX_TO_PTR(dst)));
  return dst;
}

static DST_accessibility
DwarfAccess(AccessSpecifier as) {
  switch (as) {
    case AS_private:   return DW_ACCESS_private;
    case AS_protected: return DW_ACCESS_protected;
    default:           return DW_ACCESS_public;
  }
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForRecordType(const RecordType *type) {
  TRACE_FUNC();
  ASTContext *ctx = _builder->Context();
  const RecordDecl *rd = type->getDecl();
  const RecordDecl *def = rd->getDefinition();
  const Decl *key = def ? def : rd->getCanonicalDecl();
  ST_DST_MAP::iterator it = _sym_map.find(key);
  if (it != _sym_map.end())
    return it->second;

  std::string name = rd->getName().str();
  USRCPOS src = GetSrcPos(rd->getLocation());
  DST_size_t size = def ? ctx->getTypeSize(type) / 8 : 0;
  DST_INFO_IDX dst = rd->isUnion()
    ? DST_mk_union_type(src, const_cast<char *>(name.c_str()), size,
                        DST_INVALID_IDX, def == NULL)
    : DST_mk_structure_type(src, const_cast<char *>(name.c_str()), size,
                            DST_INVALID_IDX, def == NULL);
  AppendType(dst);
  // before the members, which may point back to this type
  _sym_map[key] = dst;
  if (!def || def->isInvalidDecl())
    return dst;

  const ASTRecordLayout &layout = ctx->getASTRecordLayout(def);
  if (const CXXRecordDecl *cxx = dyn_cast<CXXRecordDecl>(def)) {
    for (const CXXBaseSpecifier &base : cxx->bases()) {
      if (base.isVirtual())
        continue;   // TODO (C++, phase 3)
      const CXXRecordDecl *bd = base.getType()->getAsCXXRecordDecl();
      if (!bd)
        continue;
      DST_append_child(dst,
        DST_mk_inheritance(src, CreateDstForType(base.getType()),
                           DW_VIRTUALITY_none,
                           layout.getBaseClassOffset(bd).getQuantity(),
                           DwarfAccess(base.getAccessSpecifier())));
    }
  }
  unsigned idx = 0;
  for (RecordDecl::field_iterator f = def->field_begin();
       f != def->field_end(); ++f, ++idx) {
    std::string fname = f->getName().str();
    QualType ftype = f->getType();
    // unnamed bit-fields are padding; anonymous structs and unions stay
    if (fname.empty() && !ftype->isRecordType())
      continue;
    UINT64 bitoff = layout.getFieldOffset(idx);
    DST_INFO_IDX fdst = CreateDstForType(ftype);
    USRCPOS fsrc = GetSrcPos(f->getLocation());
    DST_accessibility access = DwarfAccess(f->getAccess());
    DST_INFO_IDX mdst;
    if (f->isBitField()) {
      unsigned width = f->getBitWidthValue(*ctx);
      if (width == 0)
        continue;
      // As wgen's DST_enter_normal_field: the container is the field's
      // type, at the alignment boundary at or before the field; the bit
      // offset counts from the container's most significant bit.
      UINT64 tsize = ctx->getTypeSize(ftype) / 8;
      UINT64 align = ctx->getTypeAlign(ftype) / 8;
      if (align == 0)
        align = 1;
      UINT64 byteoff = bitoff / 8;
      UINT64 container = byteoff - byteoff % align;
      while (container * 8 + tsize * 8 < bitoff + width)
        container += align;   // a field that crosses its boundary (packed)
      UINT64 into = bitoff - container * 8;
      if (Target_Is_Little_Endian)
        into = tsize * 8 - into - width;
      mdst = DST_mk_member(fsrc, const_cast<char *>(fname.c_str()), fdst,
                           container, tsize, into, width,
                           TRUE, FALSE, FALSE, FALSE, access);
    } else {
      mdst = DST_mk_member(fsrc, const_cast<char *>(fname.c_str()), fdst,
                           bitoff / 8, 0, 0, 0,
                           FALSE, FALSE, FALSE, FALSE, access);
    }
    DST_append_child(dst, mdst);
  }
  return dst;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForEnumType(const EnumType *type) {
  TRACE_FUNC();
  ASTContext *ctx = _builder->Context();
  const EnumDecl *ed = type->getDecl();
  const EnumDecl *def = ed->getDefinition();
  const Decl *key = def ? def : ed->getCanonicalDecl();
  ST_DST_MAP::iterator it = _sym_map.find(key);
  if (it != _sym_map.end())
    return it->second;
  std::string name = ed->getName().str();
  DST_size_t size = def ? ctx->getTypeSize(type) / 8 : 0;
  DST_INFO_IDX dst =
    DST_mk_enumeration_type(GetSrcPos(ed->getLocation()),
                            const_cast<char *>(name.c_str()), size,
                            DST_INVALID_IDX, def == NULL);
  AppendType(dst);
  _sym_map[key] = dst;
  if (!def)
    return dst;
  for (const EnumConstantDecl *ec : def->enumerators()) {
    DST_CONST_VALUE val;
    INT64 v = ec->getInitVal().getSExtValue();
    if (size == 8) {
      DST_CONST_VALUE_form(val) = DST_FORM_DATA8;
      DST_CONST_VALUE_form_data8(val) = v;
    } else {
      DST_CONST_VALUE_form(val) = DST_FORM_DATA4;
      DST_CONST_VALUE_form_data4(val) = (INT32) v;
    }
    std::string ename = ec->getName().str();
    DST_append_child(dst, DST_mk_enumerator(GetSrcPos(ec->getLocation()),
                                            const_cast<char *>(ename.c_str()),
                                            val));
  }
  return dst;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForTypedef(const TypedefType *type) {
  TRACE_FUNC();
  const TypedefNameDecl *td = type->getDecl();
  ST_DST_MAP::iterator it = _sym_map.find(td);
  if (it != _sym_map.end())
    return it->second;
  std::string name = td->getName().str();
  DST_INFO_IDX under = CreateDstForType(td->getUnderlyingType());
  DST_INFO_IDX dst = DST_mk_typedef(GetSrcPos(td->getLocation()),
                                    const_cast<char *>(name.c_str()),
                                    under, DST_INVALID_IDX);
  AppendType(dst);
  _sym_map[td] = dst;
  return dst;
}

DST_INFO_IDX
WhirlDstBuilder::CreateDstForType(const QualType T, TY_IDX ty_idx) {
  TRACE_FUNC();
  if (T.isNull() || !WantFullDebugInfo())
    return DST_INVALID_IDX;
  const void *key = T.getAsOpaquePtr();
  TYPE_DST_MAP::iterator it = _type_map.find(key);
  if (it != _type_map.end())
    return it->second;

  ASTContext *ctx = _builder->Context();
  DST_INFO_IDX dst_idx = DST_INVALID_IDX;
  Qualifiers quals = T.getLocalQualifiers();
  if (quals.hasConst()) {
    QualType unq = T;
    unq.removeLocalConst();
    dst_idx = AppendType(DST_mk_const_type(CreateDstForType(unq)));
  } else if (quals.hasVolatile()) {
    QualType unq = T;
    unq.removeLocalVolatile();
    dst_idx = AppendType(DST_mk_volatile_type(CreateDstForType(unq)));
  } else if (quals.hasRestrict()) {
    QualType unq = T;
    unq.removeLocalRestrict();
    dst_idx = CreateDstForType(unq);
  } else {
    const Type *type = T.getTypePtr();
    switch (type->getTypeClass()) {
      case Type::Builtin:
        dst_idx = CreateDstForBuiltinType(cast<BuiltinType>(type));
        break;
      case Type::Complex: {
        QualType elt = cast<ComplexType>(type)->getElementType();
        std::string name = "complex " + elt.getAsString(ctx->getPrintingPolicy());
        std::map<std::string, DST_INFO_IDX>::iterator bt = _base_types.find(name);
        if (bt != _base_types.end())
          dst_idx = bt->second;
        else {
          dst_idx = AppendType(DST_mk_basetype(name.c_str(),
                                               elt->isIntegerType() ? DW_ATE_signed
                                                                    : DW_ATE_complex_float,
                                               ctx->getTypeSize(type) / 8));
          _base_types[name] = dst_idx;
        }
        break;
      }
      case Type::Pointer:
      case Type::BlockPointer:
        // (a struct is entered before its members, so a pointer back to
        // it terminates)
        dst_idx = AppendType(DST_mk_pointer_type(
                               CreateDstForType(type->getPointeeType()),
                               DW_ADDR_none, ctx->getTypeSize(type) / 8));
        break;
      case Type::LValueReference:
      case Type::RValueReference:
        dst_idx = AppendType(DST_mk_reference_type(
                               CreateDstForType(type->getPointeeType()),
                               DW_ADDR_none, ctx->getTypeSize(type) / 8));
        break;
      case Type::ConstantArray:
      case Type::IncompleteArray:
      case Type::VariableArray:
        dst_idx = CreateDstForArrayType(cast<ArrayType>(type));
        break;
      case Type::Vector:
      case Type::ExtVector:
        dst_idx = CreateDstForVectorType(cast<VectorType>(type));
        break;
      case Type::FunctionNoProto:
      case Type::FunctionProto:
        dst_idx = CreateDstForFunctionType(cast<FunctionType>(type));
        break;
      case Type::Record:
        dst_idx = CreateDstForRecordType(cast<RecordType>(type));
        break;
      case Type::Enum:
        dst_idx = CreateDstForEnumType(cast<EnumType>(type));
        break;
      case Type::Typedef:
        dst_idx = CreateDstForTypedef(cast<TypedefType>(type));
        break;
      case Type::Atomic:
        dst_idx = CreateDstForType(cast<AtomicType>(type)->getValueType());
        break;
      case Type::Decayed:
        dst_idx = CreateDstForType(cast<DecayedType>(type)->getDecayedType());
        break;
      default:
        // sugar (elaborated, parenthesized, typeof, attributed, ...)
        {
          QualType inner = type->getLocallyUnqualifiedSingleStepDesugaredType();
          if (inner.getAsOpaquePtr() != key)
            dst_idx = CreateDstForType(inner);
        }
        break;
    }
  }
  _type_map[key] = dst_idx;
  return dst_idx;
}

} // namespace wgen
