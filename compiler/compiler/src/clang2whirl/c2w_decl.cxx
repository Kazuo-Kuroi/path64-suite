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

#include <functional>
#include <algorithm>
#include <clang/AST/VTableBuilder.h>
#include <clang/AST/VTTBuilder.h>
#include "c2w_decl.h"
#include "c2w_builder.h"
#include "c2w_func.h"
#include "c2w_stmt.h"
#include "c2w_tracer.h"

// clang header files
#include "clanginc.h"

using namespace clang;

// open64 header files
#include "open64inc.h"
#include "c2w_expr.h"

std::set<const clang::VarDecl *> _deferred_global_var_decls;
std::set<int> _global_var_priority; //priorities for global var

extern "C" void
dump_decl(void* decl) {
  ((Decl*)decl)->dump();
}

namespace wgen {

GlobalDecl
GetGlobalDecl(const FunctionDecl *decl) {
  if (const CXXConstructorDecl *ctor = dyn_cast<CXXConstructorDecl>(decl))
    return GlobalDecl(ctor,  CXXCtorType::Ctor_Complete);
  if (const CXXDestructorDecl *dtor = dyn_cast<CXXDestructorDecl>(decl))
    return GlobalDecl(dtor, CXXDtorType::Dtor_Complete);
  return GlobalDecl(decl);
}

GlobalDecl
GetGlobalDecl(const FunctionDecl *decl, GlobalDecl gd) {
  if (const CXXConstructorDecl *ctor = dyn_cast<CXXConstructorDecl>(decl))
    return GlobalDecl(ctor, gd.getCtorType());
  if (const CXXDestructorDecl *dtor = dyn_cast<CXXDestructorDecl>(decl))
    return GlobalDecl(dtor, gd.getDtorType());
  return GlobalDecl(decl);
}

WhirlDeclBuilder::WhirlDeclBuilder(WhirlBuilder *builder)
  : _builder(builder) {
}

WhirlDeclBuilder::~WhirlDeclBuilder() {
}

void
WhirlDeclBuilder::Initialize() {
  TRACE_FUNC();
}

void
WhirlDeclBuilder::Finalize() {
  TRACE_FUNC();

  // process _alias_list
  if (!_alias_list.empty()) {
    for (IDTYPE i = 1; i < ST_Table_Size(GLOBAL_SYMTAB); ++i) {
      ST* st_base = &St_Table(GLOBAL_SYMTAB,i);
      if (ST_class(st_base) != CLASS_VAR &&
          ST_class(st_base) != CLASS_FUNC)
        continue;
      AliasList::iterator it;
      StringRef name(ST_name(st_base));
      for (it = _alias_list.begin(); it != _alias_list.end(); ) {
        if (name.compare(it->second->getAliasee()) != 0) {
          ++it;
          continue;
        }
        {
          ST_IDX st_idx = _builder->SB().GetST(it->first);
          if (st_idx != ST_IDX_ZERO) {
            ST* st = ST_ptr(st_idx);
            // As wgen's WGEN_Assemble_Alias: a weak alias is an extern
            // weak symbol naming its strong one; any other alias shares
            // the target's storage.
            if (it->first->hasAttr<WeakAttr>())
              Set_ST_is_weak_symbol(st);
            if (ST_is_weak_symbol(st)) {
              Set_ST_sclass(st, SCLASS_EXTERN);
              Set_ST_strong_idx(*st, ST_st_idx(st_base));
            }
            else {
              Set_ST_base_idx(st, ST_st_idx(st_base));
              Set_ST_emit_symbol(st);
              Set_ST_sclass(st, ST_sclass(st_base));
              if (ST_is_initialized(st_base))
                Set_ST_is_initialized(st);
              if (ST_init_value_zero(st_base))
                Set_ST_init_value_zero(st);
            }
            if (ST_sym_class(st) != ST_sym_class(st_base)) {
              Set_ST_class(st, ST_class(st_base));
              Set_ST_type(st, ST_type(st_base));
            }
            // the target may have no other use
            if (ST_sym_class(st_base) == CLASS_FUNC)
              Set_PU_no_delete(Pu_Table[ST_pu(st_base)]);
          }
          // done; a target can have several aliases, so keep looking
          it = _alias_list.erase(it);
          if (_alias_list.empty())
            return;
        }
      }
    }
  }
  // process the rest _alias_list
  AliasList::iterator it;
  for (it = _alias_list.begin(); it != _alias_list.end(); ++it) {
    ST_IDX st_idx = _builder->SB().GetST(it->first);
    if (st_idx != ST_IDX_ZERO) {
      ST* st = ST_ptr(st_idx);
      Set_ST_is_weak_symbol(st);

      TY_IDX ty = ST_type(st);

      if (ST_class(st) == CLASS_FUNC) {
        PU_IDX pu_idx;
        PU &pu = New_PU(pu_idx);
        PU_Init(pu, ty, GLOBAL_SYMTAB + 1);
        if (_builder->Lang_C())
          Set_PU_c_lang(pu);
        else if (_builder->Lang_CPP())
          Set_PU_cxx_lang(pu);
        ty = (TY_IDX) pu_idx;
      }

      STR_IDX base_str = _builder->EnterString(it->second->getAliasee());
      ST* st_base = New_ST(GLOBAL_SYMTAB);
      ST_Init(st_base, base_str, ST_class(st_idx),
              SCLASS_EXTERN, EXPORT_PREEMPTIBLE, ty);
      if (ST_is_weak_symbol(st)) {
        Set_ST_strong_idx(*st, ST_st_idx(st_base));
      }
#if 0
      else {
        Set_ST_base_idx(st, ST_st_idx(st_base));
        Set_ST_emit_symbol(st);
      }
#endif
    }
  }
}

std::string
WhirlDeclBuilder::GetMangledName(const Decl *decl, INT variant) {
  Is_True(_builder->Lang_CPP() ||
          dyn_cast<FunctionDecl>(decl) &&
          decl->getAsFunction()->hasAttr<OverloadableAttr>(),
          ("only C++ functions & overloadable functions need mangled name"));
  SmallString<256> Buffer;
  llvm::raw_svector_ostream Out(Buffer);
  MangleContext *MC = _builder->MangleContext();
  const NamedDecl *ND = cast<NamedDecl>(decl);
  if (MC->shouldMangleDeclName(ND)) {
    if (const auto *D = dyn_cast<CXXConstructorDecl>(ND))
      mangleCXXName(MC, D, (CXXCtorType)variant, Out);
    else if (const auto *D = dyn_cast<CXXDestructorDecl>(ND))
      mangleCXXName(MC, D, (CXXDtorType)variant, Out);
    else {
      // get mangled name for static guard variable
      if (variant) {
        Is_True(isa<VarDecl>(ND),
                ("only static guard variable VarDecl need mangled name"));
        MC->mangleStaticGuardVariable(cast<VarDecl>(ND), Out);
      }
      else
        MC->mangleName(ND, Out);
    }
  }
  return Out.str().str();
}

std::string
WhirlDeclBuilder::GetMangledName(const GlobalDecl gd, const ThunkInfo &thunk_info)
{
  Is_True(_builder->Lang_CPP(), ("only C++ functions needs mangled name"));
  Is_True(&thunk_info, ("missing thunk_info"));
  SmallString<256> Buffer;
  llvm::raw_svector_ostream Out(Buffer);
  MangleContext *MC = _builder->MangleContext();
  const CXXMethodDecl *ND = cast<CXXMethodDecl>(gd.getDecl());
  if (const auto *D = dyn_cast<CXXDestructorDecl>(ND))
    MC->mangleCXXDtorThunk(D, gd.getDtorType(), thunk_info.This, Out);
  else
    MC->mangleThunk(cast<CXXMethodDecl>(gd.getDecl()), thunk_info, Out);
  return Out.str().str();
}

STR_IDX
WhirlDeclBuilder::ConvertName(const Decl *decl, const llvm::StringRef &str, INT variant) {
  if (_builder->Lang_C()) {
    if (dyn_cast<FunctionDecl>(decl) &&
        decl->getAsFunction()->hasAttr<OverloadableAttr>())
      return _builder->EnterString(GetMangledName(decl, variant));
    else
      return _builder->EnterString(str);
  } else if (_builder->Lang_CPP()) {
    // Linkage specifications are transparent for a variable's namespace.
    // Let Clang decide its ABI name, including extern "C" exceptions, rather
    // than inferring linkage from the immediate declaration context.
    if (const VarDecl *var = dyn_cast<VarDecl>(decl)) {
      if (var->hasGlobalStorage() &&
          _builder->MangleContext()->shouldMangleDeclName(var))
        return _builder->EnterString(GetMangledName(decl, variant));
      return _builder->EnterString(str);
    }
    if ((decl->isFunctionOrFunctionTemplate() &&
         !cast<FunctionDecl>(decl)->isExternC() &&
         str.str() != "main") ||         /* don't mangle 'main' and extern "C" functions */
         (decl->getDeclContext()->isNamespace() &&
          !isa<ClassTemplateSpecializationDecl>(decl) &&
          !isa<CXXRecordDecl>(decl))) {  /* don't mangle class/record decl */
      return _builder->EnterString(GetMangledName(decl, variant));
    } else {
      return _builder->EnterString(str);
    }
  } else {
    Is_True(false, ("unsupport other Langs"));
    return _builder->EnterString(str);
  }
}

STR_IDX
WhirlDeclBuilder::ConvertName(const CXXConstructorDecl *decl, CXXCtorType ctor) {
  return _builder->EnterString(GetMangledName(decl, ctor));
}

STR_IDX
WhirlDeclBuilder::ConvertName(const CXXDestructorDecl *decl, CXXDtorType dtor) {
  return _builder->EnterString(GetMangledName(decl, dtor));
}

STR_IDX
WhirlDeclBuilder::ConvertName(const GlobalDecl gd, const ThunkInfo &ti) {
  return _builder->EnterString(GetMangledName(gd, ti));
}

INITV_IDX
WhirlDeclBuilder::ConvertAPValue(QualType type, const APValue *value, INT repeat) {
  INITV_IDX inv = INITV_IDX_ZERO;
  switch (value->getKind()) {
  default:
    Is_True(FALSE, ("bad kind"));
    break;
  case APValue::ValueKind::Int:
    {
      TY_IDX ty = _builder->TB().ConvertType(type);
      Is_True(MTYPE_is_integral(TY_mtype(ty)), ("bad ty"));
      inv = New_INITV();
      INITV_Init_Integer(inv, TY_mtype(ty), value->getInt().getLimitedValue(), repeat);
      break;
    }
  case APValue::ValueKind::Float:
    {
      TY_IDX ty = _builder->TB().ConvertType(type);
      Is_True(MTYPE_is_float(TY_mtype(ty)), ("bad ty"));
      TCON_IDX tcon = Convert_float_to_tcon(ty, value->getFloat());
      inv = New_INITV();
      INITV_Set_VAL(Initv_Table[inv], tcon, repeat);
      break;
    }
  case APValue::ValueKind::ComplexInt:
    Is_True(FALSE, ("TODO: support integer complex"));
    break;
  case APValue::ValueKind::ComplexFloat:
    {
      TY_IDX ty = _builder->TB().ConvertType(type);
      Is_True(MTYPE_is_complex(TY_mtype(ty)), ("bad ty"));
      TCON_IDX tcon = Convert_complex_float_to_tcon(ty,
                                                    value->getComplexFloatReal(),
                                                    value->getComplexFloatImag());
      inv = New_INITV();
      INITV_Set_VAL(Initv_Table[inv], tcon, repeat);
      break;
    }
  case APValue::ValueKind::LValue:
    {
      TY_IDX ty = _builder->TB().ConvertType(type);
      Is_True(TY_kind(ty) == KIND_POINTER ||
              TY_kind(ty) == KIND_ARRAY, ("bad ty"));
      UINT64 ofst = value->getLValueOffset().getQuantity();
      const APValue::LValueBase base = value->getLValueBase();
      if (!base && TY_kind(ty) == KIND_POINTER) {
        inv = New_INITV();
        INITV_Init_Integer(inv, TY_mtype(ty), ofst, repeat);
      }
      else {
        if (const ValueDecl *vd = base.dyn_cast<const ValueDecl*>()) {
          Is_True(TY_kind(ty) == KIND_POINTER, ("bad ty"));
          ST_IDX st = _builder->Get_decl_st(vd);
          Is_True(st != ST_IDX_ZERO, ("bad st"));
          inv = New_INITV();
          INITV_Init_Symoff(inv, &St_Table[st], ofst, repeat);
        }
#if LLVM_VERSION_MAJOR == 11
        // handle typeid
        else if (TypeInfoLValue ti = base.dyn_cast<TypeInfoLValue>()) {
          ST_IDX st = _builder->TB().ConvertRTTIForType(QualType(ti.getType(), 0));
          Is_True(st != ST_IDX_ZERO, ("rtti st is zero"));
          inv = New_INITV();
          INITV_Init_Symoff(inv, ST_ptr(st), 0, repeat);
        }
#endif
        else if (const Expr *expr = base.dyn_cast<const Expr*>()) {
          Is_True(ofst == 0, ("TODO: ofst is not 0"));
          WhirlConstBuilder const_bldr(_builder);
          Result r = const_bldr.ConvertConst(expr);
          Is_True(r.isSym() || r.isTCon() || r.isInitV() || r.isIntConst(),
                  ("bad return kind"));
          Is_True(r.Ty() != TY_IDX_ZERO, ("invalid return type"));
          switch (r.Kind()) {
          case R_ST:
            inv = New_INITV();
            INITV_Init_Symoff(inv, ST_ptr(r.Sym()), 0, repeat);
            break;
          case R_TCON:
            Is_True(TY_kind(r.Ty()) == KIND_ARRAY, ("not array type"));
            inv = New_INITV();
            if (TY_kind(ty) == KIND_ARRAY) {
              const TCON &tcon = Tcon_Table[r.Tcon()];
              Is_True(TCON_ty(tcon) == MTYPE_STRING, ("bad tcon ty?"));
              INT64 pad = TY_size(ty) - TCON_str_len(tcon);
              if (pad > 0) {
                INITV_Init_Block(inv, INITV_Next_Idx(), repeat);
                INITV_IDX cnt_inv = New_INITV();
                INITV_Set_VAL(Initv_Table[cnt_inv], r.Tcon(), 1);
                INITV_IDX pad_inv = New_INITV();
                INITV_Init_Pad(pad_inv, pad);
                Set_INITV_next(cnt_inv, pad_inv);
              }
              else {
                INITV_Set_VAL(Initv_Table[inv], r.Tcon(), repeat);
              }
            }
            else {
              ST *cst = New_Const_Sym(r.Tcon(), r.Ty());
              INITV_Init_Symoff(inv, cst, 0, repeat);
            }
            break;
          case R_INITV:
            Is_True(TY_kind(r.Ty()) == KIND_ARRAY ||
                    TY_kind(r.Ty()) == KIND_STRUCT, ("not array type or struct type"));
            if (TY_kind(ty) == KIND_POINTER) {
              ST *st = New_ST(CURRENT_SYMTAB);
              ST_Init(st, Save_Unique_Str(".init"), CLASS_VAR, SCLASS_PSTATIC, EXPORT_LOCAL, r.Ty());
              Set_ST_is_initialized(st);
              New_INITO(ST_st_idx(st), r.InitV());
              inv = New_INITV();
              INITV_Init_Symoff(inv, st, 0, repeat);
            }
            else {
              inv = r.InitV();
            }
            break;
          case R_INTCONST:
            inv = New_INITV();
            INITV_Init_Integer(inv, TY_mtype(r.Ty()), r.IntConst(), repeat);
            break;
          default:
            Is_True(FALSE, ("not supported"));
            break;
          }
        }
        else {
          Is_True(FALSE, ("TODO: lvalue not value decl"));
        }
      }
      break;
    }
  case APValue::ValueKind::Vector:
    {
      const VectorType *vt = type->getAs<VectorType>();
      Is_True(vt != NULL, ("bad ty"));
      unsigned num_init = value->getVectorLength();
      unsigned num_vect = vt->getNumElements();
      QualType elem_ty = vt->getElementType();
      inv = New_INITV();
      INITV_Init_Block(inv, INITV_Next_Idx(), repeat);

      INITV_IDX last_inv = INITV_IDX_ZERO;
      for (int i = 0; i < num_init; ++i) {
        INITV_IDX elem_inv = ConvertAPValue(elem_ty,
                                            &value->getVectorElt(i));
        if (elem_inv == INITV_IDX_ZERO)
          continue;
        if (last_inv != INITV_IDX_ZERO)
          Set_INITV_next(last_inv, elem_inv);
        last_inv = elem_inv;
      }

      if (num_init < num_vect) {
        int pad = num_vect - num_init;
        TY_IDX ety = _builder->TB().ConvertType(elem_ty);
        INITV_IDX pad_inv = New_INITV();
        INITV_Init_Pad(pad_inv, pad * TY_size(ety));
        if (last_inv != INITV_IDX_ZERO)
          Set_INITV_next(last_inv, pad_inv);
      }
      break;
    }
  case APValue::ValueKind::Array:
    {
      const ConstantArrayType *cat = _builder->Context()->getAsConstantArrayType(type);
      Is_True(cat != NULL, ("bad type"));
      Is_True(cat->getSize().getZExtValue() == value->getArraySize(),
              ("array size mismatch?"));
      unsigned num_init = value->getArrayInitializedElts();
      unsigned num_array = cat->getSize().getZExtValue();
      QualType elem_ty = cat->getElementType();
      inv = New_INITV();
      INITV_Init_Block(inv, INITV_Next_Idx(), repeat);

      INITV_IDX last_inv = INITV_IDX_ZERO;
      for(int i = 0; i < num_init; ++i) {
        INITV_IDX elem_inv = ConvertAPValue(elem_ty,
                                            &value->getArrayInitializedElt(i));
        if (elem_inv == INITV_IDX_ZERO)
          continue;
        if (last_inv != INITV_IDX_ZERO)
          Set_INITV_next(last_inv, elem_inv);
        last_inv = elem_inv;
      }

      if (num_init < num_array) {
        int pad = num_array - num_init;
        INITV_IDX pad_inv;
        if (value->hasArrayFiller()) {
          pad_inv = ConvertAPValue(elem_ty, &value->getArrayFiller(), pad);
        }
        else {
          TY_IDX ety = _builder->TB().ConvertType(elem_ty);
          pad_inv = New_INITV();
          INITV_Init_Pad(pad_inv, pad * TY_size(ety));
        }
        if (last_inv != INITV_IDX_ZERO)
          Set_INITV_next(last_inv, pad_inv);
      }
      break;
    }
  case APValue::ValueKind::Struct:
    {
      inv = New_INITV();
      INITV_Init_Block(inv, INITV_Next_Idx(), repeat);
      INITV_IDX last_inv = INITV_IDX_ZERO;

      TY_IDX ty = _builder->TB().ConvertType(type);
      Is_True(TY_kind(ty) == KIND_STRUCT, ("not struct type"));
      FLD_ITER fld_iter = Make_fld_iter(TY_fld(ty));

      const RecordType *record_type = type->getAs<RecordType>();
      Is_True(record_type && record_type->getDecl(), ("bad type"));
      const RecordDecl *decl = record_type->getDecl();
      UINT64 emitted = 0;
      if (isa<CXXRecordDecl>(decl)) {
        const CXXRecordDecl *cxx_decl = cast<CXXRecordDecl>(decl);
        Is_True(cxx_decl->getNumBases() == value->getStructNumBases(),
                ("bases num mismatch"));
        unsigned idx = 0;
        for (CXXRecordDecl::base_class_const_iterator it = cxx_decl->bases_begin();
             it != cxx_decl->bases_end(); ++it) {
          FLD_HANDLE fld(fld_iter);
          // add pad if needed
          if (FLD_ofst(fld) - emitted > 0) {
            INITV_IDX pad_inv = New_INITV();
            INITV_Init_Pad(pad_inv, FLD_ofst(fld) - emitted);
            if (last_inv != INITV_IDX_ZERO)
              Set_INITV_next(last_inv, pad_inv);
            last_inv = pad_inv;
            emitted = FLD_ofst(fld);
          }
          if (TY_size(FLD_type(fld)) != 0) {
            // add base
            INITV_IDX base_inv = ConvertAPValue(it->getType(), &value->getStructBase(idx));
            //if (base_inv == INITV_IDX_ZERO)
            //  continue;
            Is_True(base_inv != INITV_IDX_ZERO, ("zero initv"));
            if (last_inv != INITV_IDX_ZERO)
              Set_INITV_next(last_inv, base_inv);
            last_inv = base_inv;
          }
          ++ idx;
          emitted += TY_size(FLD_type(fld));
          if (FLD_last_field(fld_iter))
            break;
          ++ fld_iter;
        }
      }

      unsigned idx = 0;
      for (RecordDecl::field_iterator it = decl->field_begin();
           it != decl->field_end(); ++it) {
        FLD_HANDLE fld(fld_iter);
        // add pad if needed
        if (FLD_ofst(fld) - emitted > 0) {
          INITV_IDX pad_inv = New_INITV();
          INITV_Init_Pad(pad_inv, FLD_ofst(fld) - emitted);
          if (last_inv != INITV_IDX_ZERO)
            Set_INITV_next(last_inv, pad_inv);
          last_inv = pad_inv;
          emitted = FLD_ofst(fld);
        }
        if (TY_size(FLD_type(fld)) != 0) {
          // add field
          const APValue fld_val = value->getStructField(idx);
          if (FLD_is_bit_field(fld)) {
            Is_True(fld_val.getKind() == APValue::ValueKind::Int,
              ("initialization value of bitfield expected to be integer"));
            AddBitfieldInitvForTree(fld_val.getInt().getLimitedValue(),
                                    fld, last_inv, emitted);
          } else {
            INITV_IDX fld_inv = ConvertAPValue(it->getType(), &fld_val);
            Is_True(fld_inv != INITV_IDX_ZERO, ("zero initv"));
            if (last_inv != INITV_IDX_ZERO)
              Set_INITV_next(last_inv, fld_inv);
            last_inv = fld_inv;
            emitted += TY_size(FLD_type(fld));
          }
        }
        ++ idx;
        if (FLD_last_field(fld_iter))
          break;
        ++ fld_iter;
      }
      Is_True(idx == value->getStructNumFields(),
              ("fields num mismatch"));
      Is_True(emitted <= TY_size(ty), ("initv larger than type size"));
      Is_True(FLD_last_field(fld_iter),
              ("field not initialized?"));
      // add pad if needed
      if (TY_size(ty) - emitted > 0) {
        INITV_IDX pad_inv = New_INITV();
        INITV_Init_Pad(pad_inv, TY_size(ty) - emitted);
        if (last_inv != INITV_IDX_ZERO)
          Set_INITV_next(last_inv, pad_inv);
      }
      break;
    }
  case APValue::ValueKind::Union:
    {
      if (value->getUnionField() == NULL)
        return INITV_IDX_ZERO;
      QualType fld_type = value->getUnionField()->getType();
      inv = ConvertAPValue(fld_type, &value->getUnionValue(), repeat);
      break;
    }
  case APValue::ValueKind::MemberPointer:
    {
      const MemberPointerType *mpt_type = type->getAs<MemberPointerType>();
      Is_True(mpt_type != NULL, ("bad type"));
      INT64 ofst = 0;
      if (value->isMemberPointerToDerivedMember()) {
        const CXXRecordDecl *from_decl = value->getMemberPointerPath().front();
        const CXXRecordDecl *to_decl = value->getMemberPointerPath().back();
        Is_True(FALSE, ("TODO: debug me"));
        ofst = ComputeOffsetHind(_builder->Context(), from_decl, to_decl);
      }
      Is_True(ofst >= 0, ("bad offset"));
      if (mpt_type->isMemberDataPointer()) {
        if (value->getMemberPointerDecl()) {
          Is_True(isa<FieldDecl>(value->getMemberPointerDecl()),
                  ("not field decl for data pointer"));
          const FieldDecl *fld = cast<FieldDecl>(value->getMemberPointerDecl());
          const ASTRecordLayout &layout = _builder->Context()->getASTRecordLayout(fld->getParent());
          ofst += layout.getFieldOffset(fld->getFieldIndex());
        }
        ofst = ofst ? ofst / MTYPE_byte_size(Pointer_Mtype) : 0;
        inv = New_INITV();
        INITV_Init_Integer(inv, MTYPE_I8, ofst, repeat);
      }
      else {
        Is_True(isa<CXXMethodDecl>(value->getMemberPointerDecl()),
                ("not method decl for  pointer"));
        inv = New_INITV();
        INITV_Init_Block(inv, INITV_Next_Idx(), repeat);
        INITV_IDX fptr_inv = New_INITV();
        const CXXMethodDecl *mtd = cast<CXXMethodDecl>(value->getMemberPointerDecl());
        GlobalDecl gd = isa<CXXDestructorDecl>(mtd)
                          ? GlobalDecl(cast<CXXDestructorDecl>(mtd), CXXDtorType::Dtor_Complete)
                          : GlobalDecl(mtd);
        if (mtd->isVirtual()) {
          ItaniumVTableContext *ctx;
          ctx = cast<ItaniumVTableContext>(_builder->Context()->getVTableContext());
          UINT64 vofst = ctx->getMethodVTableIndex(gd);
          vofst *= MTYPE_byte_size(Pointer_Mtype);
          INITV_Init_Integer(fptr_inv, MTYPE_I8, vofst + 1);
        }
        else {
          ST_IDX st = _builder->Get_func_st(gd);
          Is_True(st != ST_IDX_ZERO, ("bad st"));
          INITV_Init_Symoff(fptr_inv, ST_ptr(st), 0);
        }
        INITV_IDX adj_inv = New_INITV();
        INITV_Init_Integer(adj_inv, MTYPE_I8, ofst);
        Set_INITV_next(fptr_inv, adj_inv);
      }
      break;
    }
  case APValue::ValueKind::AddrLabelDiff:
    {
      const AddrLabelExpr *lhs = value->getAddrLabelDiffLHS();
      const AddrLabelExpr *rhs = value->getAddrLabelDiffRHS();
      Is_True(lhs && lhs->getLabel(), ("bad lhs label"));
      Is_True(rhs && rhs->getLabel(), ("bad rhs label"));
      LABEL_IDX lhs_label = _builder->Get_label_idx(lhs->getLabel());
      LABEL_IDX rhs_label = _builder->Get_label_idx(rhs->getLabel());
      Is_True(lhs_label != LABEL_IDX_ZERO && rhs_label != LABEL_IDX_ZERO,
              ("not find label from decl"));
      inv = New_INITV();
      INITV_Init_Block(inv, INITV_Next_Idx(), repeat);
      INITV_IDX lhs_inv = New_INITV();
      INITV_Init_Label(lhs_inv, lhs_label, 1);
  Set_INITV_lab_flags(lhs_inv, INITVLABELFLAGS_VALUES_FIRST);  // Path64: no label flags
      INITV_IDX rhs_inv = New_INITV();
      INITV_Init_Label(rhs_inv, rhs_label, 1);
  Set_INITV_lab_flags(rhs_inv, INITVLABELFLAGS_VALUES_LAST);  // Path64: no label flags
      Set_INITV_next(lhs_inv, rhs_inv);
      Set_LABEL_addr_saved(lhs_label);
      Set_LABEL_addr_saved(rhs_label);
      break;
    }
  }
  Is_True(inv != INITV_IDX_ZERO, ("bad inv"));
  return inv;
}

// APValue describes source fields, not ABI vptrs. Serialize a constant object
// by physical offsets, adding the complete object's address points explicitly.
static bool ContainsPolymorphicObject(ASTContext *ctx, QualType type) {
  if (const ConstantArrayType *array = ctx->getAsConstantArrayType(type))
    return ContainsPolymorphicObject(ctx, array->getElementType());
  if (const CXXRecordDecl *record = type->getAsCXXRecordDecl()) {
    if (record->isPolymorphic()) return true;
    for (const CXXBaseSpecifier &base : record->bases())
      if (ContainsPolymorphicObject(ctx, base.getType())) return true;
    for (const FieldDecl *field : record->fields())
      if (ContainsPolymorphicObject(ctx, field->getType())) return true;
  }
  return false;
}

INITV_IDX
WhirlDeclBuilder::ConvertConstantObject(QualType type, const APValue *value) {
  struct Piece { UINT64 offset, size; INITV_IDX init; };
  std::vector<Piece> pieces;
  std::map<UINT64, INITV_IDX> vptrs;
  std::map<UINT64, unsigned> bit_bytes;
  ASTContext *ctx = _builder->Context();
  const UINT64 pointer_size = MTYPE_byte_size(Pointer_Mtype);
  std::function<void(QualType, const APValue&, UINT64, bool)> emit;
  std::function<void(const FieldDecl*, const APValue&, UINT64, UINT64)> field;
  field = [&](const FieldDecl *decl, const APValue &v, UINT64 start, UINT64 bits) {
    if (!decl->isBitField()) { emit(decl->getType(), v, start + bits/8, false); return; }
    if (decl->isUnnamedBitfield() || v.getKind() == APValue::Uninitialized) return;
    const unsigned width = decl->getBitWidthValue(*ctx);
    const llvm::APSInt &integer = v.getInt();
    const bool big = ctx->getTargetInfo().isBigEndian();
    for (unsigned i=0; i<width; ++i) {
      UINT64 position = bits + i;
      unsigned source = big ? width-1-i : i;
      if (source < integer.getBitWidth() && integer[source])
        bit_bytes[start + position/8] |= 1U << (big ? 7-position%8 : position%8);
      else bit_bytes[start + position/8] |= 0;
    }
  };
  emit = [&](QualType t, const APValue &v, UINT64 start, bool base) {
    if (v.getKind() == APValue::Uninitialized) return;
    if (const ConstantArrayType *array = ctx->getAsConstantArrayType(t)) {
      QualType element = array->getElementType();
      UINT64 size = ctx->getTypeSizeInChars(element).getQuantity();
      for (unsigned i=0; i<v.getArraySize(); ++i) {
        if (i<v.getArrayInitializedElts()) emit(element,v.getArrayInitializedElt(i),start+i*size,false);
        else if (v.hasArrayFiller()) emit(element,v.getArrayFiller(),start+i*size,false);
      }
      return;
    }
    if (const RecordType *rt = t->getAs<RecordType>()) {
      const RecordDecl *record = rt->getDecl();
      const ASTRecordLayout &layout = ctx->getASTRecordLayout(record);
      if (v.isUnion()) {
        if (const FieldDecl *active = v.getUnionField())
          field(active, v.getUnionValue(), start, 0);
        return;
      }
      if (const CXXRecordDecl *cxx = dyn_cast<CXXRecordDecl>(record)) {
        if (!base && cxx->isDynamicClass()) {
          Is_True(!cxx->getNumVBases(), ("constant object has virtual bases"));
          ST_IDX table = GetVTableST(cxx);
          ItaniumVTableContext *tables = cast<ItaniumVTableContext>(ctx->getVTableContext());
          const VTableLayout &vt = tables->getVTableLayout(cxx);
          for (const auto &address : vt.getAddressPoints()) {
            UINT64 off = start + address.first.getBaseOffset().getQuantity();
            if (vptrs.count(off)) continue;
            UINT64 index = vt.getVTableOffset(address.second.VTableIndex) +
                           address.second.AddressPointIndex;
            INITV_IDX init = New_INITV();
            INITV_Init_Symoff(init, ST_ptr(table), index*pointer_size);
            vptrs[off] = init;
          }
        }
        unsigned i=0;
        for (const CXXBaseSpecifier &b : cxx->bases()) {
          const CXXRecordDecl *br = b.getType()->getAsCXXRecordDecl();
          emit(b.getType(),v.getStructBase(i++),
               start+layout.getBaseClassOffset(br).getQuantity(),true);
        }
      }
      unsigned i=0;
      for (const FieldDecl *f : record->fields()) {
        field(f,v.getStructField(i),start,layout.getFieldOffset(i));
        ++i;
      }
      return;
    }
    INITV_IDX init = ConvertAPValue(t,&v);
    if (init) pieces.push_back({start,UINT64(ctx->getTypeSizeInChars(t).getQuantity()),init});
  };
  emit(type,*value,0,false);
  for (const auto &vptr : vptrs) pieces.push_back({vptr.first,pointer_size,vptr.second});
  for (const auto &byte : bit_bytes) {
    INITV_IDX init = New_INITV(); INITV_Init_Integer(init,MTYPE_U1,byte.second);
    pieces.push_back({byte.first,1,init});
  }
  std::sort(pieces.begin(),pieces.end(),[](const Piece &a,const Piece &b) { return a.offset<b.offset; });
  INITV_IDX first=0,last=0;
  UINT64 used=0,total=ctx->getTypeSizeInChars(type).getQuantity();
  auto append = [&](INITV_IDX init) { if(last) Set_INITV_next(last,init); else first=init; last=init; };
  for (const Piece &piece : pieces) {
    Is_True(piece.offset>=used, ("overlapping constant object initializers"));
    if (piece.offset>used) { INITV_IDX pad=New_INITV(); INITV_Init_Pad(pad,piece.offset-used); append(pad); }
    append(piece.init); used=piece.offset+piece.size;
  }
  Is_True(used<=total, ("constant object initializer too large"));
  if (used<total) { INITV_IDX pad=New_INITV(); INITV_Init_Pad(pad,total-used); append(pad); }
  INITV_IDX block=New_INITV(); INITV_Init_Block(block,first);
  return block;
}

ST_IDX
WhirlDeclBuilder::GetStaticArrayDtor(const VarDecl *decl) {
  auto &symbol = _static_array_dtors[decl->getCanonicalDecl()];
  if (!symbol) symbol = ReserveDtorWrapperSymbol(_builder->GetSrcPos());
  return symbol;
}

void
WhirlDeclBuilder::EmitStaticArrayDtors() {
  const TranslationUnitDecl *unit = _builder->Context()->getTranslationUnitDecl();
  while (!_static_array_dtors.empty()) {
    const auto entry = *_static_array_dtors.begin();
    _static_array_dtors.erase(_static_array_dtors.begin());
    ScopeHelper<const TranslationUnitDecl*> scope(_builder->Scope(),unit);
    WhirlFuncBuilder function(_builder);
    ST_IDX symbol = entry.second;
    function.EmitDtorPtr(entry.first,symbol);
  }
}

void
WhirlDeclBuilder::CreateInitoInitvEntry(const VarDecl *decl, ST_IDX st_idx) {
  TRACE_FUNC();
  Is_True(st_idx != (ST_IDX) 0, ("symbol must have created"));
  Is_True(decl->hasInit(), ("no decl init"));
#if 0
  TYPE_ID mtype;
  UINT64 val;
  UINT size;
  if (!decl->hasInit())
    return;
  ST *st = ST_ptr(st_idx);
  if ((ST_sclass(st_idx) == SCLASS_UGLOBAL && !ST_init_value_zero(st)) ||
      ST_sclass(st_idx) == SCLASS_EXTERN ||
      ST_sclass(st_idx) == SCLASS_COMMON)
    Set_ST_sclass(st, SCLASS_DGLOBAL);
  if (!ST_is_initialized(st))
    Set_ST_is_initialized(st);
  INITV_IDX inv = INITV_IDX_ZERO;

  QualType type = decl->getType();
  const APValue *eva_val = decl->evaluateValue();

  // create INITV for evaluated value
  if (eva_val) {
    inv = ConvertAPValue(type, eva_val);
    Is_True(inv != INITV_IDX_ZERO, ("bad initv"));
    New_INITO(st_idx, inv);
    return;
#if 0
    TY_IDX ty_idx = _builder->TB().ConvertType(type);
    if (eva_val->isInt() &&
        MTYPE_is_integral(TY_mtype(ty_idx))) {
      inv = New_INITV();
      INITV_Init_Integer(inv, TY_mtype(ty_idx), eva_val->getInt().getZExtValue());
      New_INITO(st_idx, inv);
      return;
    } else if (eva_val->isFloat()) {
      TCON_IDX tcon = Convert_float_to_tcon(ty_idx, eva_val->getFloat());
      inv = New_INITV();
      INITV_Set_VAL(Initv_Table[inv], tcon, 1);
      New_INITO(st_idx, inv);
      return;
    }
    else if (eva_val->isLValue() && TY_kind(ty_idx) == KIND_POINTER) {
      UINT64 ofst = eva_val->getLValueOffset().getQuantity();
      if (!eva_val->isNullPointer()) {
        const APValue::LValueBase base = eva_val->getLValueBase();
        if (const ValueDecl *VD = base.dyn_cast<const ValueDecl*>()) {
          ST_IDX init_st = _builder->SB().ConvertSymbol(VD);
          inv = New_INITV();
          INITV_Init_Symoff(inv, &St_Table[init_st], ofst);
          New_INITO(st_idx, inv);
          return;
        }
        // fall through to process expr
      }
      else {
        inv = New_INITV();
        INITV_Init_Integer(inv, TY_mtype(ty_idx), ofst);
        New_INITO(st_idx, inv);
        return;
      }
    }
#endif
  }
#endif

  ST *st = ST_ptr(st_idx);
  TY_IDX ty_idx = _builder->TB().ConvertType(decl->getType());
  // no need to generate INITV when ty size is zero
  if (TY_size(ty_idx) == 0) {
    Set_ST_init_value_zero(st);
    return;
  }

  INITV_IDX inv = INITV_IDX_ZERO;

  WhirlConstBuilder const_bldr(_builder);
  Result r = const_bldr.ConvertConstForDecl(decl);
  if (r.isTCon()) {
    inv = New_INITV();
    INITV_Set_VAL(Initv_Table[inv], r.Tcon(), 1);

    // add padding for StringLiteral init
    if (isa<StringLiteral>(decl->getInit())) {
      TY_IDX ty_idx = ST_type(st_idx);
      Is_True(TY_kind(ty_idx) == KIND_ARRAY, ("Strings should always be arrays"));
      TCON tcon = Tcon_Table[r.Tcon()];
      mUINT32 len = Targ_String_Length (tcon);
      if (len < TY_size(ty_idx)) {
        INITV_IDX pad = New_INITV();
        INITV_Init_Pad(pad, TY_size(ty_idx) - len);
        Set_INITV_next(inv, pad);
      }
    }
  } else if (r.isInitV()) {
    inv = r.InitV();
  } else if (r.isIntConst() ||
             (r.isNode() && WN_operator(r.Node()) == OPR_INTCONST)) {
    UINT64 val = r.isNode() ? WN_const_val(r.Node()) : r.IntConst();
    // For C, don't set INIT_VALUE_ZERO for symbols with assigned
    // sections to ensure they remain DGLOBAL, and get assigned to
    // data section.
    if (val == 0 && (_builder->Lang_CPP() || !ST_has_named_section(st))) {
      Set_ST_init_value_zero(st);
      if (ST_sclass(st) == SCLASS_DGLOBAL ||
          ST_sclass(st) == SCLASS_COMMON)
        Set_ST_sclass(st, SCLASS_UGLOBAL);
      return;
    }

    inv = New_INITV();
    INITV_Init_Integer(inv, TY_mtype(ty_idx), val);
  } else if (r.isSym()) {
    inv = New_INITV();
    INITV_Init_Symoff(inv, &St_Table[r.Sym()], 0);
  } else {
    Is_True(false, ("Can't go to here."));
  }
  New_INITO(st_idx, inv);
}

void
WhirlDeclBuilder::ConvertAccessSpec(const AccessSpecDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertAccessSpec"));
}

void
WhirlDeclBuilder::ConvertCaptured(const CapturedDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertCaptured"));
}

void
WhirlDeclBuilder::ConvertClassScopeFunctionSpecialization(const ClassScopeFunctionSpecializationDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertClassScopeFunctionSpecialization"));
}

void
WhirlDeclBuilder::ConvertClassTemplatePartialSpecialization(const ClassTemplatePartialSpecializationDecl *decl) {
  TRACE_FUNC();
  ConvertCXXRecord(cast<CXXRecordDecl>(decl));
}

void
WhirlDeclBuilder::ConvertClassTemplateSpecialization(const ClassTemplateSpecializationDecl *decl) {
  TRACE_FUNC();
  ConvertCXXRecord(cast<CXXRecordDecl>(decl));
}

void
WhirlDeclBuilder::ConvertCXXConstructor(const CXXConstructorDecl *decl, CXXCtorType ctor) {
  TRACE_FUNC();
  ConvertFunction(GlobalDecl(decl, ctor));
  if (ctor == CXXCtorType::Ctor_Complete && decl->doesThisDeclarationHaveABody() &&
      decl->isExternallyVisible() && !decl->isInlined())
    _builder->AddDeferredFunc(GlobalDecl(decl, CXXCtorType::Ctor_Base), true);
}

void
WhirlDeclBuilder::ConvertCXXConversion(const CXXConversionDecl *decl) {
  TRACE_FUNC();
  ConvertFunction(GlobalDecl(decl));
}

void
WhirlDeclBuilder::ConvertCXXDestructor(const CXXDestructorDecl *decl, CXXDtorType dtor) {
  TRACE_FUNC();
  ConvertFunction(GlobalDecl(decl, dtor));
  if (dtor == CXXDtorType::Dtor_Complete && decl->doesThisDeclarationHaveABody() &&
      decl->isExternallyVisible() && !decl->isInlined()) {
    _builder->AddDeferredFunc(GlobalDecl(decl, CXXDtorType::Dtor_Base), true);
    if (decl->isVirtual())
      _builder->AddDeferredFunc(GlobalDecl(decl, CXXDtorType::Dtor_Deleting), true);
  }
}

void
WhirlDeclBuilder::ConvertCXXMethod(const CXXMethodDecl *decl) {
  TRACE_FUNC();
  if (isa<CXXConstructorDecl>(decl) || isa<CXXDestructorDecl>(decl))
    return;

  ConvertFunction(GlobalDecl(decl));
}

BOOL
WhirlDeclBuilder::HasVTable(const clang::CXXRecordDecl *decl) {
  if (!decl->hasDefinition() ||
      (!decl->isPolymorphic() && !decl->getNumVBases()))
    return FALSE;
  return TRUE;
}

BOOL
WhirlDeclBuilder::ShouldEmitVTable(const clang::CXXRecordDecl *decl) {
  Is_True(HasVTable(decl), ("no vtable for record"));
  // TSK_ExplicitInstantiationDeclaration, vtable is defined externally
  TemplateSpecializationKind decl_tsk = decl->getTemplateSpecializationKind();
  if (decl_tsk == TSK_ExplicitInstantiationDeclaration)
    return FALSE;

  // TSK_ImplicitInstantiation and TSK_ExplicitInstantiationDefinition
  // vtable must be defined here
  if (decl_tsk == TSK_ImplicitInstantiation ||
      decl_tsk == TSK_ExplicitInstantiationDefinition)
    return TRUE;

  // if no key function, define vtable here
  const CXXMethodDecl *kfunc = _builder->Context()->getCurrentKeyFunction(decl);
  if (kfunc == NULL)
    return TRUE;

  // vtable should be defined where the key function is defined
  return _builder->GetFunctionDefinition(kfunc) != NULL;
}

INITV_IDX
WhirlDeclBuilder::ConvertVTableComponent(const VTableLayout &layout, unsigned idx, unsigned &thunk_idx) {
  VTableComponent component = layout.vtable_components()[idx];
  INITV_IDX inv = New_INITV();
  switch(component.getKind()) {
    case VTableComponent::CK_VCallOffset:
      INITV_Init_Integer(inv, MTYPE_I8, component.getVCallOffset().getQuantity());
      break;
    case VTableComponent::CK_VBaseOffset:
      INITV_Init_Integer(inv, MTYPE_I8, component.getVBaseOffset().getQuantity());
      break;
    case VTableComponent::CK_OffsetToTop: {
      // FIXME: type is not fit for system
      INITV_Init_Integer(inv, MTYPE_I8, component.getOffsetToTop().getQuantity());
      break;
    }
    case VTableComponent::CK_RTTI: {
      {
        const CXXRecordDecl *decl = component.getRTTIDecl();
        if (decl != NULL) {
          QualType qtype(decl->getTypeForDecl(), 0);
          ST_IDX st_idx = _builder->TB().ConvertRTTIForType(qtype);
          Is_True(st_idx != ST_IDX_ZERO, ("rtti st is zero"));
          INITV_Init_Symoff(inv, &St_Table[st_idx], 0);
        }
        else {
          INITV_Init_Integer(inv, MTYPE_I8, 0);
        }
      }
      break;
    }
    case VTableComponent::CK_FunctionPointer:
    case VTableComponent::CK_CompleteDtorPointer:
    case VTableComponent::CK_DeletingDtorPointer: {
      GlobalDecl GD = getVTableComponentGlobalDecl(component);
      // Pure virtual member functions.
      if (cast<CXXMethodDecl>(GD.getDecl())->isPure()) {
        ST_IDX st_idx = _builder->SB().EmitCXXPureVirtualST();
        INITV_Init_Symoff(inv, &St_Table[st_idx], 0);
      } else if (cast<CXXMethodDecl>(GD.getDecl())->isDeleted()) {
        Is_True(false, ("do not support delete virtual method."));
      } else if (thunk_idx < layout.vtable_thunks().size() &&
                 layout.vtable_thunks()[thunk_idx].first == idx) {
        // thunk method
        ThunkInfo ti = layout.vtable_thunks()[thunk_idx].second;
        thunk_idx++;
        ST_IDX sym_idx = _builder->Get_thunk_st(GD, ti);
        Is_True(sym_idx != (ST_IDX)0, ("Invalid symbol idx"));
        INITV_Init_Symoff(inv, &St_Table[sym_idx], 0);
      } else {
        const CXXMethodDecl *decl = dyn_cast<CXXMethodDecl>(GD.getDecl());
        Is_True(decl != NULL, ("GD decl is not NamedDecl, decl name : %s", GD.getDecl()->getDeclKindName()));
        ST_IDX sym_idx = _builder->Get_func_st(GD);
        Is_True(sym_idx != ST_IDX_ZERO, ("bad st"));
        INITV_Init_Symoff(inv, &St_Table[sym_idx], 0);
      }
      break;
    }
    case VTableComponent::CK_UnusedFunctionPointer:
      INITV_Init_Integer(inv, MTYPE_I8, 0);
      break;
    default: {
      Is_True(false, ("not supported vtable component kind : %d.", component.getKind()));
      break;
    }
  }
  return inv;
}

ST_IDX
WhirlDeclBuilder::GetVTableST(const clang::CXXRecordDecl *decl) {
  TY_IDX record_type_idx = _builder->TB().ConvertType(decl->getTypeForDecl());
  ST_IDX sym_idx = TY_vtable(record_type_idx);
  if (sym_idx == ST_IDX_ZERO) {
    sym_idx = _builder->SB().VSC().Get(decl);
    Is_True(!ST_is_initialized(&St_Table[sym_idx]),
            ("vtable st initialized"));
    Set_TY_vtable(record_type_idx, sym_idx);
    Set_ST_vtable_ty_idx(&St_Table[sym_idx], record_type_idx);
    _builder->AddDeferredVTable(decl);
  }
  return sym_idx;
}

WN *
WhirlDeclBuilder::GetVTTArgument(const CXXRecordDecl *base, INT64 offset) {
  const CXXMethodDecl *method = cast<CXXMethodDecl>(_builder->Scope().CurrentDecl());
  const CXXRecordDecl *record = method->getParent();
  VTTBuilder layout(*_builder->Context(), record, true);
  UINT64 index = 0;
  if (base != record) {
    auto position = layout.getSubVTTIndicies().find(
        BaseSubobject(base, CharUnits::fromQuantity(offset)));
    Is_True(position != layout.getSubVTTIndicies().end(), ("missing sub-VTT"));
    index = position->second;
  }
  WN *pointer;
  ST_IDX incoming = _builder->Scope().Get_vtt();
  if (incoming)
    pointer = WN_Ldid(Pointer_Mtype, 0, incoming, ST_type(incoming));
  else {
    ST *symbol = ST_ptr(GetVTTST(record));
    Set_ST_addr_saved(symbol);
    Set_ST_addr_passed(symbol);
    pointer = WN_Lda(Pointer_Mtype, 0, symbol);
  }
  return WN_Add(Pointer_Mtype, pointer,
                WN_Intconst(Pointer_Mtype, index * MTYPE_byte_size(Pointer_Mtype)));
}

ST_IDX
WhirlDeclBuilder::GetVTTST(const CXXRecordDecl *decl) {
  ST_IDX result = _builder->SB().VTTSC().Get(decl);
  if (ST_is_initialized(ST_ptr(result)))
    return result;
  Set_ST_is_initialized(ST_ptr(result));

  ASTContext &context = *_builder->Context();
  VTTBuilder builder(context, decl, true);
  ItaniumVTableContext *vtables =
    cast<ItaniumVTableContext>(context.getVTableContext());
  ItaniumMangleContext *mangler =
    cast<ItaniumMangleContext>(_builder->MangleContext());
  std::vector<ST_IDX> symbols;
  std::vector<const VTableLayout *> layouts;
  std::vector<std::unique_ptr<VTableLayout> > construction_layouts;
  for (const VTTVTable &table : builder.getVTTVTables()) {
    if (table.getBase() == decl && table.getBaseOffset().isZero()) {
      symbols.push_back(GetVTableST(decl));
      layouts.push_back(&vtables->getVTableLayout(decl));
      continue;
    }
    construction_layouts.push_back(vtables->createConstructionVTableLayout(
        table.getBase(), table.getBaseOffset(), table.isVirtual(), decl));
    const VTableLayout &layout = *construction_layouts.back();
    SmallString<256> name;
    llvm::raw_svector_ostream output(name);
    mangler->mangleCXXCtorVTable(decl, table.getBaseOffset().getQuantity(),
                               table.getBase(), output);
    TY_IDX type = Make_Array_Type(Pointer_Mtype, 1,
                                  layout.vtable_components().size());
    ST *symbol = New_ST(GLOBAL_SYMTAB);
    ST_Init(symbol, Save_Str(output.str().str().c_str()), CLASS_VAR,
            SCLASS_DGLOBAL, EXPORT_PREEMPTIBLE, type);
    Set_ST_is_weak_symbol(symbol);
    Set_ST_is_const_var(symbol);
    Set_ST_is_initialized(symbol);
    INITV_IDX block = New_INITV();
    INITV_Init_Block(block, INITV_Next_Idx());
    INITV_IDX last = 0;
    unsigned thunk = 0;
    for (unsigned i = 0; i < layout.vtable_components().size(); ++i) {
      INITV_IDX component = ConvertVTableComponent(layout, i, thunk);
      if (last)
        Set_INITV_next(last, component);
      last = component;
    }
    New_INITO(ST_st_idx(symbol), block);
    symbols.push_back(ST_st_idx(symbol));
    layouts.push_back(&layout);
  }

  INITV_IDX block = New_INITV();
  INITV_Init_Block(block, INITV_Next_Idx());
  INITV_IDX last = 0;
  for (const VTTComponent &component : builder.getVTTComponents()) {
    const VTableLayout &layout = *layouts[component.VTableIndex];
    VTableLayout::AddressPointLocation address =
      layout.getAddressPoint(component.VTableBase);
    UINT64 index = layout.getVTableOffset(address.VTableIndex) + address.AddressPointIndex;
    INITV_IDX value = New_INITV();
    INITV_Init_Symoff(value, ST_ptr(symbols[component.VTableIndex]),
                      index * MTYPE_byte_size(Pointer_Mtype));
    if (last)
      Set_INITV_next(last, value);
    last = value;
  }
  New_INITO(result, block);
  return result;
}

void
WhirlDeclBuilder::ConvertVTable(const clang::CXXRecordDecl *decl) {
  TY_IDX record_type_idx = _builder->TB().ConvertType(decl->getTypeForDecl());
  ST_IDX sym_idx = TY_vtable(record_type_idx);
  Is_True(sym_idx != ST_IDX_ZERO &&
          ST_vtable_ty_idx(&St_Table[sym_idx]) == record_type_idx &&
          !ST_is_initialized(&St_Table[sym_idx]),
          ("vtable st not created or already generated"));

  ASTContext *ast_c = _builder->Context();
  ItaniumVTableContext *vtable_c = cast<ItaniumVTableContext>(ast_c->getVTableContext());
  const VTableLayout &layout = vtable_c->getVTableLayout(decl);
  unsigned thunk_idx = 0;
  INITV_IDX inv_blk = New_INITV();
  INITV_Init_Block(inv_blk, INITV_Next_Idx());
  INITV_IDX last = 0;
  for (unsigned i = 0; i < layout.getNumVTables(); i++) {
    size_t begin = layout.getVTableOffset(i);
    size_t size = layout.getVTableSize(i);
    for (unsigned j = begin; j < begin + size; j++) {
      INITV_IDX cur = ConvertVTableComponent(layout, j, thunk_idx);
      if (last != 0) {
        Set_INITV_next(last, cur);
      }
      last = cur;
    }
  }
  Set_ST_sclass(&St_Table[sym_idx], SCLASS_DGLOBAL);
  Set_ST_is_initialized(&St_Table[sym_idx]);
  New_INITO(sym_idx, inv_blk);
}

void
WhirlDeclBuilder::ConvertCXXRecord(const CXXRecordDecl *decl) {
  TRACE_FUNC();

  if (!decl->hasDefinition() /* || decl->getCanonicalDecl() != decl */)
    return;

  for (auto *mem_decl : cast<CXXRecordDecl>(decl)->decls()) {
    if (isa<VarDecl>(mem_decl) ||
        isa<CXXRecordDecl>(mem_decl)) {
      _builder->EmitTopLevelDecl(mem_decl);
    }
    else if (isa<FunctionDecl>(mem_decl)) {
      _builder->AddFunctionDecl(cast<FunctionDecl>(mem_decl));
    }
  }
}

void
WhirlDeclBuilder::ConvertEnum(const EnumDecl *decl) {
  TRACE_FUNC();
  if (decl->isCompleteDefinition() || decl->isFixed()) {
    _builder->TB().ConvertType(decl->getIntegerType());
    return;
  }
  // TODO: handle uncompleted enum typ
}

void
WhirlDeclBuilder::ConvertEnumConstant(const EnumConstantDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertEnumConstant"));
}

void
WhirlDeclBuilder::ConvertField(const FieldDecl *decl) {
  TRACE_FUNC();
}

void
WhirlDeclBuilder::ConvertFileScopeAsm(const FileScopeAsmDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertFileScopeAsm"));
}

void
WhirlDeclBuilder::ConvertFriend(const FriendDecl *decl) {
  TRACE_FUNC();
}

void
WhirlDeclBuilder::ConvertFriendTemplate(const FriendTemplateDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertFriendTemplate"));
}

void
WhirlDeclBuilder::HandleAttrs(const NamedDecl *decl, ST_IDX st_idx) {
  Is_True(decl->hasAttrs(), ("decl has no attrs"));
    const AttrVec &attrs = decl->getAttrs();
    for (AttrVec::const_iterator it = attrs.begin();
         it != attrs.end(); ++it) {
      switch ((*it)->getKind()) {
      case attr::Alias: {
          const AliasAttr* attr = cast<AliasAttr>(*it);
          _alias_list.push_back(std::make_pair(decl, attr));
        }
        break;
      case attr::WeakRef:
        if (st_idx != ST_IDX_ZERO)
          Set_ST_is_weak_symbol(ST_ptr(st_idx));
        break;
      case attr::Weak:
        // Also on a redeclaration; a weak definition is never common.
        if (st_idx != ST_IDX_ZERO) {
          Set_ST_is_weak_symbol(ST_ptr(st_idx));
          if (ST_sclass(ST_ptr(st_idx)) == SCLASS_COMMON)
            Set_ST_sclass(ST_ptr(st_idx), SCLASS_UGLOBAL);
        }
        break;
      default:
        //TRACE_WARN(("Unsupported attributes"));
        break;
      }
    }
}

BOOL
WhirlDeclBuilder::ConvertFunction(GlobalDecl gd) {
  TRACE_FUNC();

  const FunctionDecl *decl = cast<FunctionDecl>(gd.getDecl());
  // Path64: a use names the most recent declaration, which need not be
  // the one with the body (C99: "inline int f() {...}" followed by
  // "extern inline int f();"). Convert the definition.
  if (!decl->doesThisDeclarationHaveABody() &&
      !isa<CXXConstructorDecl>(decl) && !isa<CXXDestructorDecl>(decl)) {
    const FunctionDecl *def = NULL;
    if (decl->hasBody(def) && def->doesThisDeclarationHaveABody()) {
      decl = def;
      gd = GlobalDecl(def);
    }
  }
  ST_IDX st_idx = (ST_IDX) 0;
  if (decl->doesThisDeclarationHaveABody() ||
      decl->doesDeclarationForceExternallyVisibleDefinition() ||
      decl->hasAttr<AliasAttr>()) {
    // generate function declaration (an alias needs its symbol too)
    st_idx = _builder->SB().ConvertSymbol(gd);
  }

  if (decl->hasAttrs())
    HandleAttrs(decl, st_idx);

  if (!decl->doesThisDeclarationHaveABody()) {
    if (!decl->getDeclContext()->isRecord())
      return FALSE;
    else
      if (decl->isTrivial() || st_idx == (ST_IDX) 0)
        return FALSE;
  }

  // defer ConvertFunction if Current_Map_Tab already initialized
  if (isa<FunctionDecl>(decl) && Current_Map_Tab) {
    _builder->AddDeferredFunc(gd);
    return FALSE;
  }

  // generate function body
  if (_builder->IsDeclHandled(gd) == FALSE) {
    Is_True(st_idx != (ST_IDX) 0,
            ("error in convertsymbol"));

    // make sure dtor call stack is empty
    FmtAssert(Is_dtor_call_stack_empty(), ("dtor stack not empty"));

    ScopeHelper<GlobalDecl> shlp(_builder->Scope(), gd);
    WhirlFuncBuilder func_bldr(_builder);
    func_bldr.ConvertFunction(gd, st_idx);
    _opaque_value_map.clear();
    return TRUE;
  }
  return FALSE;
}

void
WhirlDeclBuilder::ConvertImplicitParam(const ImplicitParamDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertImplicitParam"));
}

void
WhirlDeclBuilder::ConvertImport(const ImportDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertImport"));
}

void
WhirlDeclBuilder::ConvertIndirectField(const IndirectFieldDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertIndirectField"));
}

void
WhirlDeclBuilder::ConvertLabel(const LabelDecl *decl) {
  TRACE_FUNC();
  _builder->Get_label_idx(decl);
}

void
WhirlDeclBuilder::ConvertLinkageSpec(const LinkageSpecDecl *decl) {
  TRACE_FUNC();
  for (auto *I : decl->decls()) {
    _builder->EmitTopLevelDecl(I);
  }
}

void
WhirlDeclBuilder::ConvertMSProperty(const MSPropertyDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertMSProperty"));
}

void
WhirlDeclBuilder::ConvertNamespace(const NamespaceDecl *decl) {
  TRACE_FUNC();
  for (auto *I : decl->decls()) {
    _builder->EmitTopLevelDecl(I);
  }
}

void
WhirlDeclBuilder::ConvertNamespaceAlias(const NamespaceAliasDecl *decl) {
  TRACE_FUNC();
  // TODO: ONLY need to handle debug info for namespace alias
}

void
WhirlDeclBuilder::ConvertNonTypeTemplateParm(const NonTypeTemplateParmDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertNonTypeTemplateParm"));
}

#ifdef C2W_ENABLE_OBJC
void
WhirlDeclBuilder::ConvertObjCAtDefsField(const ObjCAtDefsFieldDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCAtDefsField"));
}

void
WhirlDeclBuilder::ConvertObjCCategory(const ObjCCategoryDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCCategory"));
}

void
WhirlDeclBuilder::ConvertObjCCategoryImpl(const ObjCCategoryImplDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCCategoryImpl"));
}

void
WhirlDeclBuilder::ConvertObjCCompatibleAlias(const ObjCCompatibleAliasDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCCompatibleAlias"));
}

void
WhirlDeclBuilder::ConvertObjCImplementation(const ObjCImplementationDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCImplementation"));
}

void
WhirlDeclBuilder::ConvertObjCInterface(const ObjCInterfaceDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCInterface"));
}

void
WhirlDeclBuilder::ConvertObjCIvar(const ObjCIvarDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCIvar"));
}

void
WhirlDeclBuilder::ConvertObjCMethod(const ObjCMethodDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCMethod"));
}

void
WhirlDeclBuilder::ConvertObjCProperty(const ObjCPropertyDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCProperty"));
}

void
WhirlDeclBuilder::ConvertObjCPropertyImpl(const ObjCPropertyImplDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCPropertyImpl"));
}

void
WhirlDeclBuilder::ConvertObjCProtocol(const ObjCProtocolDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertObjCProtocol"));
}
#endif

#ifdef C2W_ENABLE_OPENMP
void
WhirlDeclBuilder::ConvertOMPThreadPrivate(const OMPThreadPrivateDecl* decl)
{
    TRACE_FUNC();
    Is_True(false, ("unsupported ConvertOMPThreadPrivate"));
}
#endif

void
WhirlDeclBuilder::ConvertParmVar(const ParmVarDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertParmVar"));
}

void
WhirlDeclBuilder::ConvertRecord(const RecordDecl *decl) {
  TRACE_FUNC();
}

void
WhirlDeclBuilder::ConvertTemplateTemplateParm(const TemplateTemplateParmDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertTemplateTemplateParm"));
}

void
WhirlDeclBuilder::ConvertTemplateTypeParm(const TemplateTypeParmDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertTemplateTypeParm"));
}

void
WhirlDeclBuilder::ConvertTranslationUnit(const TranslationUnitDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertTranslationUnit"));
}

void
WhirlDeclBuilder::ConvertTypeAlias(const TypeAliasDecl *decl) {
  TRACE_FUNC();
  #if 1
    //TODO
  #else
  const TypedefNameDecl &type_decl = cast<TypedefNameDecl>(decl);
  QualType type = type_decl.getUnderlyingType();

  if (type->isVariablyModifiedType())
    Is_True(false, ("unsupported ConvertTypedef"));
  #endif
}

void
WhirlDeclBuilder::ConvertTypedef(const TypedefDecl *decl) {
  TRACE_FUNC();
  #if 1
    //TODO
  #else
  const TypedefNameDecl &type_decl = cast<TypedefNameDecl>(decl);
  QualType type = type_decl.getUnderlyingType();

  if (type->isVariablyModifiedType())
    Is_True(false, ("unsupported ConvertTypedef"));
  #endif
}

void
WhirlDeclBuilder::ConvertUnresolvedUsingTypename(const UnresolvedUsingTypenameDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertUnresolvedUsingTypename"));
}

void
WhirlDeclBuilder::ConvertUnresolvedUsingValue(const UnresolvedUsingValueDecl *decl) {
  TRACE_FUNC();
  Is_True(false, ("unsupported ConvertUnresolvedUsingValue"));
}

void
WhirlDeclBuilder::ConvertUsing(const UsingDecl *decl) {
  TRACE_FUNC();
  #if 1
  // TODO: only need for debuginfo?
  #else
  Is_True(false, ("unsupported ConvertUsing"));
  #endif
}

void
WhirlDeclBuilder::ConvertUsingDirective(const UsingDirectiveDecl *decl) {
  TRACE_FUNC();
  #if 1
  // TODO: only need for debuginfo?
  #else 
  Is_True(false, ("unsupported ConvertUsingDirective"));
  #endif
}

// return TRUE for static initialization
static BOOL
Ignore_expr_for_static(const Expr *expr) {
  Is_True(expr != NULL, ("invalid Expr"));
  if (isa<CallExpr>(expr) ||
      isa<CXXBindTemporaryExpr>(expr) ||
      isa<CXXTypeidExpr>(expr) ||
      isa<DeclRefExpr>(expr) ||
      isa<ExprWithCleanups>(expr) ||
      isa<MaterializeTemporaryExpr>(expr) ||
      isa<MemberExpr>(expr) ||
      isa<CXXNewExpr>(expr))
    return FALSE;
  return TRUE;
}

// return TRUE for a static initialization
// return FALSE for a dynamic initialization
// if no_need_initialize is TRUE, nothing need to do
BOOL
WhirlDeclBuilder::IsStaticInitialized(const Expr *init_expr, bool &no_need_initialize) {

  // remove wrappers out of "core" init expr like
  // ExprWithCleanups/InitListExpr
  if (init_expr && isa<ExprWithCleanups>(init_expr))
    // ExprWithCleanups
    init_expr = cast<ExprWithCleanups>(init_expr)->getSubExpr();

  // The initializer_list holds a pointer to separately materialized storage.
  if (isa<CXXStdInitializerListExpr>(init_expr))
    return FALSE;

  if (isa<InitListExpr>(init_expr)) {
    int num_element = 0;
    const CXXRecordDecl *record =
      ConvertConstantArrayType(init_expr->getType(), num_element);
    // for constant array type, if record has no trivial destructor,
    // it should be dynamic initialized, which could call
    // the destructor call. so if all the init value is constant 
    // we will do dynamic init instead of creating global INITO
    if (record && !record->hasTrivialDestructor())
      return FALSE;

    // no need to initialize for NULL InitList
    if (cast<InitListExpr>(init_expr)->getNumInits() == 0) {
      no_need_initialize = TRUE;
      return FALSE;
    }

    std::vector<const Expr*> elems;
    CollectInitListItems(cast<InitListExpr>(init_expr), elems);
    for (UINT i = 0; i <elems.size(); ++i) {
      if (!elems[i]) continue; //StringLiteral
      if (elems[i] && isa<ImplicitCastExpr>(elems[i])) {
        CastKind kind = dyn_cast<ImplicitCastExpr>(elems[i])->getCastKind();
        if (kind == CK_ArrayToPointerDecay ||
            kind == CK_FunctionToPointerDecay)
          continue;
      }
      if (isa<ImplicitValueInitExpr>(elems[i]))
        continue;
      if (!IsStaticInitialized(elems[i], no_need_initialize))
        return FALSE;
    }
    return TRUE;
  }

  // deferred global var decls
  if (isa<CXXConstructExpr>(init_expr)) {
    CXXRecordDecl *record =
      init_expr->getType()->getBaseElementTypeUnsafe()->getAsCXXRecordDecl();
    bool needs_global_ctor = FALSE;
    bool needs_global_dtor = record && record->hasDefinition() &&
                             !record->hasTrivialDestructor();

    if (record->isLambda())
      return FALSE;
    if (init_expr = GetNonTrivialInitializer(init_expr))
      needs_global_ctor = TRUE;
    if (needs_global_ctor || needs_global_dtor) {
      // defer CXXConstructorDecl if not exist
      if (needs_global_ctor) {
        Is_True(isa<CXXConstructExpr>(init_expr), ("invalid init_expr"));
        const CXXConstructExpr *ctor_expr = cast<CXXConstructExpr>(init_expr);
        const CXXConstructorDecl *ctor_decl = ctor_expr->getConstructor();
        if (!ctor_decl->isTrivial())
          _builder->Get_func_st(GlobalDecl(ctor_decl, CXXCtorType::Ctor_Complete));
      }
      // defer CXXDestructorDecl if not exist
      if (needs_global_dtor) {
        const CXXDestructorDecl *dtor_decl = record->getDestructor();
        if (dtor_decl && !dtor_decl->isTrivial())
          _builder->Get_func_st(GlobalDecl(dtor_decl, CXXDtorType::Dtor_Complete));
      }
    } else {
      // no need initialize
      no_need_initialize = TRUE;
    }
    return FALSE;
  }

#if 0
  if (isa<CastExpr>(init_expr)) {
    const CastExpr *expr = cast<CastExpr>(init_expr);
    CastKind ck = expr->getCastKind();
    const Expr *sub_expr = expr->getSubExpr();
    if ((ck == CK_ArrayToPointerDecay ||
         ck == CK_FunctionToPointerDecay) &&
        isa<DeclRefExpr>(sub_expr))
      return TRUE;
  }

  init_expr = init_expr->IgnoreParenCasts();
  // handle UnaryOperator
  if (isa<UnaryOperator>(init_expr)) {
    const UnaryOperator *expr = cast<UnaryOperator>(init_expr);
    const Expr *sub_expr = expr->getSubExpr();
    if (expr->getOpcode() == UO_AddrOf &&
        isa<DeclRefExpr>(sub_expr))
      return TRUE;
    init_expr = sub_expr->IgnoreParenCasts();
  }
  if (!Ignore_expr_for_static(init_expr))
    return FALSE;

  // handle BinaryOperator
  if (isa<BinaryOperator>(init_expr)) {
    const Expr *lhs = cast<BinaryOperator>(init_expr)->getLHS();
    if (!IsStaticInitialized(lhs, no_need_initialize))
      return FALSE;
    const Expr *rhs = cast<BinaryOperator>(init_expr)->getRHS();
    if (!IsStaticInitialized(rhs, no_need_initialize))
      return FALSE;
  }
#endif
  return TRUE;
}

// True if a constant initializer is all zero bits, so the variable can go
// in .bss as wgen puts it (an explicit "= 0" or "= {0}" included).
static bool
APValueIsZero(const APValue &v) {
  switch (v.getKind()) {
  case APValue::Uninitialized:
    return true;
  case APValue::Int:
    return v.getInt() == 0;
  case APValue::Float:
    return v.getFloat().isPosZero();
  case APValue::ComplexInt:
    return v.getComplexIntReal() == 0 && v.getComplexIntImag() == 0;
  case APValue::ComplexFloat:
    return v.getComplexFloatReal().isPosZero() &&
           v.getComplexFloatImag().isPosZero();
  case APValue::LValue:
    return v.isNullPointer() && !v.getLValueBase() &&
           v.getLValueOffset().isZero();
  case APValue::Vector:
    for (unsigned i = 0; i < v.getVectorLength(); i++)
      if (!APValueIsZero(v.getVectorElt(i)))
        return false;
    return true;
  case APValue::Array:
    for (unsigned i = 0; i < v.getArrayInitializedElts(); i++)
      if (!APValueIsZero(v.getArrayInitializedElt(i)))
        return false;
    return !v.hasArrayFiller() || APValueIsZero(v.getArrayFiller());
  case APValue::Struct:
    for (unsigned i = 0; i < v.getStructNumBases(); i++)
      if (!APValueIsZero(v.getStructBase(i)))
        return false;
    for (unsigned i = 0; i < v.getStructNumFields(); i++)
      if (!APValueIsZero(v.getStructField(i)))
        return false;
    return true;
  case APValue::Union:
    return !v.getUnionField() || APValueIsZero(v.getUnionValue());
  default:   // member pointers, label differences
    return false;
  }
}

// A variable whose storage never changes: const, not volatile, and in
// C++ no mutable member or destructor (wgen: TREE_READONLY). With a
// static initializer it goes in .rodata.
static bool
IsReadOnlyVar(ASTContext *ctx, const VarDecl *decl) {
  QualType type = decl->getType();
  if (type.isVolatileQualified() ||
      !ctx->getBaseElementType(type).isConstQualified())
    return false;
  // (QualType::isConstantStorage, which Clang 4.0 lacks.)
  if (const CXXRecordDecl *rd = ctx->getBaseElementType(type)->getAsCXXRecordDecl())
    if (rd->hasDefinition() &&
        (rd->hasMutableFields() || !rd->hasTrivialDestructor()))
      return false;
  return true;
}

void
WhirlDeclBuilder::ConvertVar(const VarDecl *decl) {
  TRACE_FUNC();
  ST_IDX st_idx = _builder->SB().GetST(decl);
  // st already converted
  if (st_idx != ST_IDX_ZERO &&
      (ST_is_initialized(ST_ptr(st_idx)) ||
       (ST_sclass(st_idx) != SCLASS_EXTERN &&
        ST_sclass(st_idx) != SCLASS_UGLOBAL &&
        ST_sclass(st_idx) != SCLASS_FSTATIC)))
    return;

  if (st_idx == ST_IDX_ZERO)
    st_idx = _builder->SB().ConvertSymbol(decl);

  // An in-class initializer does not supply storage for a non-inline static
  // member. Keep it external until its out-of-class definition is seen.
  if (decl->isStaticDataMember() && !decl->isThisDeclarationADefinition()) {
    const VarDecl *definition = decl->getDefinition();
    if (!definition) return;
    decl = definition;
  }
  ST *st = ST_ptr(st_idx);
  // reset st_idx's sclass & eclass after redecllaration
  if (ST_sclass(st) == SCLASS_EXTERN && !decl->hasExternalStorage() &&
      (decl->isThisDeclarationADefinition() || decl->getDefinition())) {
    Set_ST_sclass(st, _builder->SB().IsCommonVar(decl) ? SCLASS_COMMON
                                                         : SCLASS_UGLOBAL);
    if (ST_export(st) != EXPORT_LOCAL)
      Set_ST_export(st, _builder->SB().VisibilityExport(decl, true));
  }

  if (decl->hasAttrs())
    HandleAttrs(decl, st_idx);

  // return prematurely if decl has no init to avoid redeclarations
  const VarDecl *initializer_decl = decl;
  const Expr *initializer = decl->getAnyInitializer(initializer_decl);
  if (!initializer || isa<CXXNullPtrLiteralExpr>(initializer))
    return;

  // reset st_idx's sclass if decl has init
  if (ST_sclass(st) == SCLASS_FSTATIC &&
      ST_export(st) == EXPORT_LOCAL &&
      decl->hasExternalStorage()) {
    Set_ST_sclass(st, SCLASS_COMMON);
    Set_ST_export(st, EXPORT_PREEMPTIBLE);
  }

  // reset st type, if ST_type(st_idx) is incomplete
  QualType type = decl->getType();
  TY_IDX ty_idx = _builder->TB().ConvertType(type);
  TY_IDX tmp_ty = ST_type(st);
  if (ty_idx != tmp_ty && TY_is_incomplete(tmp_ty) && !TY_is_incomplete(ty_idx))
    Set_ST_type(st, ty_idx);

  if (decl->isLocalVarDecl() && !decl->isStaticLocal())
    return;

  const Expr *init_expr = decl->getAnyInitializer(decl);
  if (decl->hasGlobalStorage() && init_expr) {
    // reset EXTERN storage to COMMON
    if (ST_sclass(st) == SCLASS_EXTERN)
      Set_ST_sclass(st, SCLASS_COMMON);
  }

  const APValue *eva_val = decl->evaluateValue();
  // Keep constant vptr relocations in the object even when another compiler's
  // inline guard performs only destructor registration.
  if (_builder->Lang_CPP() && eva_val &&
      ContainsPolymorphicObject(_builder->Context(), type)) {
    New_INITO(st, ConvertConstantObject(type,eva_val));
    if (IsReadOnlyVar(_builder->Context(),decl)) Set_ST_is_const_var(st);
    if (ST_sclass(st)==SCLASS_UGLOBAL || ST_sclass(st)==SCLASS_COMMON ||
        ST_sclass(st)==SCLASS_EXTERN) Set_ST_sclass(st,SCLASS_DGLOBAL);
    Set_ST_is_initialized(st);
    const CXXRecordDecl *record = _builder->Context()->getBaseElementType(type)->getAsCXXRecordDecl();
    if (record && !record->hasTrivialDestructor()) {
      _builder->Get_func_st(GlobalDecl(record->getDestructor(),CXXDtorType::Dtor_Complete));
      if (!decl->isStaticLocal()) _deferred_global_var_decls.insert(decl);
    }
    return;
  }
  bool no_need_initialize = FALSE;
  if (_builder->Lang_CPP()) {
    if (eva_val == NULL || !IsStaticInitialized(init_expr, no_need_initialize)) {
      // do dynamic initialization for CPP
      if (!decl->isStaticLocal())
        _deferred_global_var_decls.insert(decl);
      return;
    }
  }

  if (IsReadOnlyVar(_builder->Context(), decl))
    Set_ST_is_const_var(st);

  if (eva_val != NULL) {
    if (TY_size(ty_idx) == 0)
      return;

    // An all-zero initializer goes in .bss, as for no initializer. (In
    // C, not for a variable with a section: it stays where it was put.)
    if (APValueIsZero(*eva_val) &&
        (_builder->Lang_CPP() || !ST_has_named_section(st))) {
      Set_ST_init_value_zero(st);
      Set_ST_is_initialized(st);
      if (ST_sclass(st) == SCLASS_DGLOBAL || ST_sclass(st) == SCLASS_COMMON ||
          ST_sclass(st) == SCLASS_EXTERN)
        Set_ST_sclass(st, SCLASS_UGLOBAL);
      return;
    }

    INITV_IDX inv = ConvertAPValue(type, eva_val);
    if (inv == INITV_IDX_ZERO)
      return;
    New_INITO(st, inv);
  } else {
    // for C
    if (_builder->Lang_C())
      CreateInitoInitvEntry(decl, st_idx);
  }

  if (!ST_init_value_zero(st) &&
      (ST_sclass(st) == SCLASS_UGLOBAL ||
       ST_sclass(st) == SCLASS_EXTERN ||
       ST_sclass(st) == SCLASS_COMMON))
    Set_ST_sclass(st, SCLASS_DGLOBAL);

  if (!ST_is_initialized(st))
    Set_ST_is_initialized(st);

#if 0
  bool no_need_initialize = FALSE;
  bool init_is_static = IsStaticInitialized(init_expr, no_need_initialize);

  // no need to initialize
  if (no_need_initialize)
    return;

  if (init_is_static)
    CreateInitoInitvEntry(decl, st_idx);
  else
    if (!decl->isStaticLocal())
      _deferred_global_var_decls.insert(decl);
#endif
}

void
WhirlDeclBuilder::ConvertDecl(const Decl *decl) {
  TRACE_FUNC();
// TODO
// require to handle imcomplete decl for types, functions, extern symbols
  switch (decl->getKind()) {
    case Decl::AccessSpec:
      ConvertAccessSpec(cast<AccessSpecDecl>(decl));
      break;
    // No code generaation needed
    case Decl::Block:
    case Decl::ClassTemplate:
    case Decl::Empty:
    case Decl::FunctionTemplate:
    case Decl::UsingShadow:
    case Decl::StaticAssert:
    case Decl::TypeAliasTemplate:
    case Decl::VarTemplate:
    case Decl::VarTemplatePartialSpecialization:
      break;
    case Decl::Captured:
      ConvertCaptured(cast<CapturedDecl>(decl));
      break;
    case Decl::ClassScopeFunctionSpecialization:
      ConvertClassScopeFunctionSpecialization(cast<ClassScopeFunctionSpecializationDecl>(decl));
      break;
    case Decl::ClassTemplatePartialSpecialization:
      ConvertClassTemplatePartialSpecialization(cast<ClassTemplatePartialSpecializationDecl>(decl));
      break;
    case Decl::ClassTemplateSpecialization:
      ConvertClassTemplateSpecialization(cast<ClassTemplateSpecializationDecl>(decl));
      break;
    case Decl::CXXConstructor:
      ConvertCXXConstructor(cast<CXXConstructorDecl>(decl),
                            CXXCtorType::Ctor_Complete);
      break;
    case Decl::CXXConversion:
      ConvertCXXConversion(cast<CXXConversionDecl>(decl));
      break;
    case Decl::CXXDestructor:
      ConvertCXXDestructor(cast<CXXDestructorDecl>(decl),
                           CXXDtorType::Dtor_Complete);
      break;
    case Decl::CXXMethod:
      ConvertCXXMethod(cast<CXXMethodDecl>(decl));
      break;
    case Decl::CXXRecord:
      ConvertCXXRecord(cast<CXXRecordDecl>(decl));
      break;
    case Decl::Enum:
      ConvertEnum(cast<EnumDecl>(decl));
      break;
    case Decl::EnumConstant:
      ConvertEnumConstant(cast<EnumConstantDecl>(decl));
      break;
    case Decl::Field:
      ConvertField(cast<FieldDecl>(decl));
      break;
    case Decl::FileScopeAsm:
      ConvertFileScopeAsm(cast<FileScopeAsmDecl>(decl));
      break;
    case Decl::Friend:
      ConvertFriend(cast<FriendDecl>(decl));
      break;
    case Decl::FriendTemplate:
      ConvertFriendTemplate(cast<FriendTemplateDecl>(decl));
      break;
    case Decl::Function:
      ConvertFunction(GlobalDecl(cast<FunctionDecl>(decl)));
      break;
    case Decl::ImplicitParam:
      ConvertImplicitParam(cast<ImplicitParamDecl>(decl));
      break;
    case Decl::Import:
      ConvertImport(cast<ImportDecl>(decl));
      break;
    case Decl::IndirectField:
      ConvertIndirectField(cast<IndirectFieldDecl>(decl));
      break;
    case Decl::Label:
      ConvertLabel(cast<LabelDecl>(decl));
      break;
    case Decl::LinkageSpec:
      ConvertLinkageSpec(cast<LinkageSpecDecl>(decl));
      break;
    case Decl::MSProperty:
      ConvertMSProperty(cast<MSPropertyDecl>(decl));
      break;
    case Decl::Namespace:
      ConvertNamespace(cast<NamespaceDecl>(decl));
      break;
    case Decl::NamespaceAlias:
      ConvertNamespaceAlias(cast<NamespaceAliasDecl>(decl));
      break;
    case Decl::NonTypeTemplateParm:
      ConvertNonTypeTemplateParm(cast<NonTypeTemplateParmDecl>(decl));
      break;
#ifdef C2W_ENABLE_OBJC
    case Decl::ObjCAtDefsField:
        ConvertObjCAtDefsField(cast<ObjCAtDefsFieldDecl>(decl));
        break;
    case Decl::ObjCCategory:
        ConvertObjCCategory(cast<ObjCCategoryDecl>(decl));
        break;
    case Decl::ObjCCategoryImpl:
        ConvertObjCCategoryImpl(cast<ObjCCategoryImplDecl>(decl));
        break;
    case Decl::ObjCCompatibleAlias:
        ConvertObjCCompatibleAlias(cast<ObjCCompatibleAliasDecl>(decl));
        break;
    case Decl::ObjCImplementation:
        ConvertObjCImplementation(cast<ObjCImplementationDecl>(decl));
        break;
    case Decl::ObjCInterface:
        ConvertObjCInterface(cast<ObjCInterfaceDecl>(decl));
        break;
    case Decl::ObjCIvar:
        ConvertObjCIvar(cast<ObjCIvarDecl>(decl));
        break;
    case Decl::ObjCMethod:
        ConvertObjCMethod(cast<ObjCMethodDecl>(decl));
        break;
    case Decl::ObjCProperty:
        ConvertObjCProperty(cast<ObjCPropertyDecl>(decl));
        break;
    case Decl::ObjCPropertyImpl:
        ConvertObjCPropertyImpl(cast<ObjCPropertyImplDecl>(decl));
        break;
    case Decl::ObjCProtocol:
        ConvertObjCProtocol(cast<ObjCProtocolDecl>(decl));
        break;
#endif
#ifdef C2W_ENABLE_OPENMP
    case Decl::OMPThreadPrivate:
        ConvertOMPThreadPrivate(cast<OMPThreadPrivateDecl>(decl));
        break;
#endif
    case Decl::ParmVar:
      ConvertParmVar(cast<ParmVarDecl>(decl));
      break;
    case Decl::Record:
      ConvertRecord(cast<RecordDecl>(decl));
      break;
    case Decl::TemplateTemplateParm:
      ConvertTemplateTemplateParm(cast<TemplateTemplateParmDecl>(decl));
      break;
    case Decl::TemplateTypeParm:
      ConvertTemplateTypeParm(cast<TemplateTypeParmDecl>(decl));
      break;
    case Decl::TranslationUnit:
      ConvertTranslationUnit(cast<TranslationUnitDecl>(decl));
      break;
    case Decl::TypeAlias:
      ConvertTypeAlias(cast<TypeAliasDecl>(decl));
      break;
    case Decl::Typedef:
      ConvertTypedef(cast<TypedefDecl>(decl));
      break;
    case Decl::UnresolvedUsingTypename:
      ConvertUnresolvedUsingTypename(cast<UnresolvedUsingTypenameDecl>(decl));
      break;
    case Decl::UnresolvedUsingValue:
      ConvertUnresolvedUsingValue(cast<UnresolvedUsingValueDecl>(decl));
      break;
    case Decl::Using:
      ConvertUsing(cast<UsingDecl>(decl));
      break;
    case Decl::UsingDirective:
      ConvertUsingDirective(cast<UsingDirectiveDecl>(decl));
      break;
    case Decl::Var:
    case Decl::VarTemplateSpecialization:
      ConvertVar(cast<VarDecl>(decl));
      break;
    default:
      Is_True(false, ("unknown decl: %s", decl->getDeclKindName()));
  }
}

const ST_IDX
WhirlDeclBuilder::GetTcfST(const Decl *decl) {
  const clang::Decl *canon_decl = decl->getCanonicalDecl();
  ST_IDX_MAP::iterator it = _dtor_tcf_map.find(canon_decl);
  if (it != _dtor_tcf_map.end())
    return it->second;
  return (ST_IDX)0;
}

const CXXRecordDecl *
WhirlDeclBuilder::ConvertConstantArrayType(
  QualType type, int &num_element) {
  const CXXRecordDecl *record;

  if (const ConstantArrayType *cat =
        _builder->Context()->getAsConstantArrayType(type)) {
    QualType subtype = cat->getElementType();
    num_element = cat->getSize().getZExtValue();
    if (subtype->isConstantArrayType()) {
      int sub_element = 0;
      record = ConvertConstantArrayType(subtype, sub_element);
      num_element *= sub_element;
    }
    else {
      record = subtype->getAsCXXRecordDecl();
    }
  }
  else {
    record = type->getAsCXXRecordDecl();
  }

  return record;
}

void
WhirlDeclBuilder::CollectInitListItems(const InitListExpr *init, std::vector<const Expr *>& elems) {
  UINT cnt = init->getNumInits();
  UINT i;
  for (i = 0; i < cnt; ++i) {
    const Expr *expr = init->getInit(i);
    if (isa<InitListExpr>(expr)  &&
        _builder->Context()->getAsConstantArrayType(expr->getType())) {
      CollectInitListItems(cast<InitListExpr>(expr), elems);
    }
    else {
      elems.push_back(expr);
    }
  }
  const ConstantArrayType *cat = _builder->Context()->getAsConstantArrayType(init->getType());
  if (cat != NULL && i < cat->getSize().getZExtValue()) {
    const Expr *filler = init->hasArrayFiller() ? init->getArrayFiller() : NULL;
    for ( ; i < cat->getSize().getZExtValue(); ++i)
      elems.push_back(filler);
  }
}

void
WhirlDeclBuilder::EmitGlobalInitializer() {
  const TranslationUnitDecl *decl = _builder->Context()->getTranslationUnitDecl();

  if (_deferred_global_var_decls.empty())
    return;

  TY_IDX ty_idx = _builder->TB().EmitGlobalInitializerType();
  PU_IDX pu_idx;
  PU &pu = New_PU(pu_idx);
  TY_IDX pu_ty_idx = ty_idx;
  PU_Init(pu, pu_ty_idx, GLOBAL_SYMTAB + 1);
  Set_PU_cxx_lang(pu);
  ty_idx = (TY_IDX) pu_idx;
  ST *st = New_ST(GLOBAL_SYMTAB);
  ST_Init (st, Save_Str("__cxx_global_var_init"), CLASS_FUNC,
           SCLASS_TEXT, EXPORT_LOCAL, ty_idx);
  Set_ST_Srcpos(*st, _builder->SetSrcPos(decl->getLocation()));

  ScopeHelper<const TranslationUnitDecl*> shlp(_builder->Scope(), decl);
  {
    for (std::set<const clang::VarDecl *>::iterator it =
           _deferred_global_var_decls.begin();
         it != _deferred_global_var_decls.end();
         ++it) {
      const VarDecl *decl = *it;

      int num_element = 0;
      const CXXRecordDecl *record =
        ConvertConstantArrayType(decl->getType(), num_element);
      if (record && !record->hasTrivialDestructor()) {
        ST_IDX tcf_st = ST_IDX_ZERO;
        WhirlFuncBuilder func_bldr(_builder);
        func_bldr.EmitDtorPtr(decl, tcf_st);
        _dtor_tcf_map[decl->getCanonicalDecl()] = tcf_st;
      }
      else if (decl->getType()->isReferenceType() && decl->getInit()) {
        // A temporary bound to the reference lives until exit: make it
        // here, with a __tcf, for MaterializeTemporaryExpr to use.
        const Expr *init = decl->getInit();
        if (const ExprWithCleanups *ewc = dyn_cast<ExprWithCleanups>(init))
          init = ewc->getSubExpr();
        while (const ImplicitCastExpr *ice = dyn_cast<ImplicitCastExpr>(init)) {
          if (ice->getCastKind() != CK_NoOp &&
              ice->getCastKind() != CK_DerivedToBase &&
              ice->getCastKind() != CK_UncheckedDerivedToBase)
            break;
          init = ice->getSubExpr();
        }
        const MaterializeTemporaryExpr *mte =
          dyn_cast<MaterializeTemporaryExpr>(init);
        const CXXRecordDecl *trec =
          mte ? mte->getType()->getAsCXXRecordDecl() : NULL;
        if (trec && !trec->hasTrivialDestructor()) {
          static int gtemp_index;
          TY_IDX tty = _builder->TB().ConvertType(mte->getType());
          ST *tmp_st = New_ST(GLOBAL_SYMTAB);
          ST_Init(tmp_st, Save_Str2i(".material.g", ".", gtemp_index++),
                  CLASS_VAR, SCLASS_FSTATIC, EXPORT_LOCAL, tty);
          _global_temp_st[mte] = ST_st_idx(tmp_st);
          ST_IDX tcf_st = ST_IDX_ZERO;
          WhirlFuncBuilder func_bldr(_builder);
          func_bldr.EmitDtorPtr(mte->getType(), ST_st_idx(tmp_st), tcf_st);
          _dtor_tcf_map[decl->getCanonicalDecl()] = tcf_st;
        }
      }

      // get priority
      int priority_size = 65535;
      if (auto *ipa = decl->getAttr<InitPriorityAttr>())
        priority_size = ipa->getPriority();

      _global_var_priority.insert(priority_size);
    }

    // emit global init func
    std::set<int>::reverse_iterator iter =
      _global_var_priority.rbegin();
    while (iter != _global_var_priority.rend()) {
      WhirlFuncBuilder func_bldr(_builder);
      func_bldr.EmitCXXGlobalInitialization(ST_st_idx(st), *iter);
      ++iter;
    }
  }

  {
    WhirlFuncBuilder func_bldr2(_builder);
    func_bldr2.EmitCXXGlobalVarDeclInitialization(ST_st_idx(st));
  }
}

// Skip over any casts and any temporary-binding expressions
static const Expr *
SkipTemporaryBindingsCastsAndParens(const Expr *expr) {
  Is_True(expr,
          ("expr should not be zero in SkipTemporaryBindingsCastsAndParens"));
  if (const MaterializeTemporaryExpr *tmp_expr = dyn_cast<MaterializeTemporaryExpr>(expr))
    expr = getSubExpr(tmp_expr);

  while (const ImplicitCastExpr *tmp_expr = dyn_cast<ImplicitCastExpr>(expr))
    expr = tmp_expr->getSubExpr();

  while (const CXXBindTemporaryExpr *tmp_expr = dyn_cast<CXXBindTemporaryExpr>(expr))
    expr = tmp_expr->getSubExpr();

  while (const CStyleCastExpr *tmp_expr = dyn_cast<CStyleCastExpr>(expr))
    expr = tmp_expr->getSubExpr();

  while (const CXXFunctionalCastExpr *tmp_expr = dyn_cast<CXXFunctionalCastExpr>(expr))
    expr = cast<CastExpr>(expr)->getSubExpr();

  while (const ImplicitCastExpr *tmp_expr = dyn_cast<ImplicitCastExpr>(expr))
    expr = tmp_expr->getSubExpr();

  return expr->IgnoreParens();
}


const Expr *
WhirlDeclBuilder::GetNonTrivialInitializer(const Expr *init) {
  Is_True(init, ("expr should not be zero in GetNonTrivialInitializer()"));
  Is_True(isa<CXXConstructExpr>(init),
          ("expr is not CXXConstructExpr: %s", init->getStmtClassName()));
  if (const CXXConstructExpr *construct = dyn_cast<CXXConstructExpr>(init)) {
    // Elide the constructor if we're constructing from a temporary
    if (construct->isElidable()) {
      const Expr *arg_expr = construct->getArg(0);
      if (arg_expr && arg_expr->isTemporaryObject(*(_builder->Context()),
                        construct->getConstructor()->getParent())) {
        arg_expr = SkipTemporaryBindingsCastsAndParens(arg_expr);
        Is_True(arg_expr, ("arg_expr should not be zero"));
        if (isa<CXXConstructExpr>(arg_expr))
          return GetNonTrivialInitializer(arg_expr);
        return init;
      } else
        return NULL;
    }
    if (CXXConstructorDecl *constructor = construct->getConstructor())
      if (constructor->isTrivial() &&
          constructor->isDefaultConstructor() &&
          !construct->requiresZeroInitialization())
        return NULL;
  }
  return init;
}

BOOL
WhirlDeclBuilder::Call_nothrow(const Decl *decl) {
  const FunctionDecl* func_decl = dyn_cast_or_null<FunctionDecl>(decl);
  if (!func_decl) {
      // Check if CapturedDecl is nothrow
      if (const CapturedDecl* captured_decl = dyn_cast_or_null<CapturedDecl>(decl))
        if (captured_decl->isNothrow())
          return TRUE;
  } else {
    const FunctionProtoType *proto = func_decl->getType()->getAs<FunctionProtoType>();
    if (proto) {
      ExceptionSpecificationType est = proto->getExceptionSpecType();
      if (isNoexceptExceptionSpec(est) && proto->canThrow(
#if LLVM_VERSION_MAJOR < 7
                                                         *_builder->Context()
#endif
                                                        ) == CT_Cannot)
        return TRUE;
      else if (est == EST_Dynamic || est == EST_DynamicNone) {
        if (!proto->getNumExceptions())
          return TRUE;
      }
    }
  }
  return FALSE;
}

ST_IDX
WhirlDeclBuilder::FindOpaqueValue(const OpaqueValueExpr *expr) {
  OPAQUE_VALUE_MAP::iterator it = _opaque_value_map.find(expr);
  if (it != _opaque_value_map.end())
    return it->second;
  return NULL;
}

void
WhirlDeclBuilder::AddOpaqueValue(const OpaqueValueExpr *expr, ST_IDX st) {
  Is_True(st != ST_IDX_ZERO, ("not a valid ST_IDX"));
  _opaque_value_map.insert(std::make_pair(expr, st));
}

} // namespace wgen
