/*
 *  TurboXSL XML+XSLT processing library
 *  Main transformation loop + interface functions
 *
 *
 *  (c) Egor Voznessenski, voznyak@mail.ru
 *
 *  $Id$
 *
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ltr_xsl.h"

#define XSL_MAIN
#include "xslglobs.h"

static void init_processor(XSLTGLOBALDATA *gctx)
{
  if(!gctx->initialized) {
    init_hash();
    xsl_stylesheet = hash("xsl:stylesheet",-1,0);
    xsl_template   = hash("xsl:template",-1,0);
    xsl_apply      = hash("xsl:apply-templates",-1,0);
    xsl_call       = hash("xsl:call-template",-1,0);
    xsl_with       = hash("xsl:with-param",-1,0);
    xsl_if         = hash("xsl:if",-1,0);
    xsl_choose     = hash("xsl:choose",-1,0);
    xsl_when       = hash("xsl:when",-1,0);
    xsl_otherwise  = hash("xsl:otherwise",-1,0);
    xsl_aimports   = hash("xsl:apply-imports",-1,0);
    xsl_element    = hash("xsl:element",-1,0);
    xsl_attribute  = hash("xsl:attribute",-1,0);
    xsl_text       = hash("xsl:text",-1,0);
    xsl_pi         = hash("xsl:processing-instruction",-1,0);
    xsl_comment    = hash("xsl:comment",-1,0);
    xsl_number     = hash("xsl:number",-1,0);
    xsl_foreach    = hash("xsl:for-each",-1,0);
    xsl_copy       = hash("xsl:copy",-1,0);
    xsl_copyof     = hash("xsl:copy-of",-1,0);
    xsl_message    = hash("xsl:message",-1,0);
    xsl_var        = hash("xsl:variable",-1,0);
    xsl_param      = hash("xsl:param",-1,0);
    xsl_fallback   = hash("xsl:fallback",-1,0);
    xsl_withparam  = hash("xsl:with-param",-1,0);
    xsl_decimal    = hash("xsl:decimal-format",-1,0);
    xsl_sort       = hash("xsl:sort",-1,0);
    xsl_strip      = hash("xsl:strip-space",-1,0);
    xsl_value_of   = hash("xsl:value-of",-1,0);
    xsl_pi         = hash("xsl:processing-instruction",-1,0);
    xsl_output     = hash("xsl:output",-1,0);
    xsl_key        = hash("xsl:key",-1,0);
    xsl_a_match    = hash("match",-1,0);
    xsl_a_select   = hash("select",-1,0);
    xsl_a_href     = hash("href",-1,0);
    xsl_a_name     = hash("name",-1,0);
    xsl_a_test     = hash("test",-1,0);
    xsl_a_mode     = hash("mode",-1,0);
    xsl_include    = hash("xsl:include",-1,0);
    xsl_import     = hash("xsl:import",-1,0);
    xsl_a_escaping = hash("disable-output-escaping",-1,0);
    xsl_a_method   = hash("method",-1,0);
    xsl_a_omitxml  = hash("omit-xml-declaration",-1,0);
    xsl_a_standalone = hash("standalone",-1,0);
    xsl_a_indent   = hash("indent",-1,0);
    xsl_a_media    = hash("media-type",-1,0);
    xsl_a_encoding = hash("encoding",-1,0);
    xsl_a_dtpublic = hash("doctype-public",-1,0);
    xsl_a_dtsystem = hash("doctype-system",-1,0);
    xsl_a_elements = hash("elements",-1,0);
    xsl_a_xmlns    = hash("xmlns",-1,0);
    xsl_a_use      = hash("use",-1,0);
    xsl_a_datatype = hash("data-type",-1,0);
    xsl_a_lang     = hash("lang",-1,0);
    xsl_a_order    = hash("order",-1,0);
    xsl_a_caseorder = hash("case-order",-1,0);
    gctx->initialized = 1;
  }
}

XMLNODE *copy_node_to_result(TRANSFORM_CONTEXT *pctx, XMLNODE *locals, XMLNODE *context, XMLNODE *parent, XMLNODE *src)
{
  XMLNODE *newnode = xml_append_child(pctx,parent,src->type);
  XMLNODE *t;
  XMLNODE *a;

  newnode->name  = src->name;
  newnode->flags = src->flags;

  //fprintf(stderr, "src->type = %i\n", src->type);

// copy attributes
  switch(src->type) {
    case ELEMENT_NODE:
      for(a=src->attributes;a;a=a->next) {
        t = xml_new_node(pctx,NULL,ATTRIBUTE_NODE);
        t->name = a->name;
        t->content = xml_process_string(pctx, locals, context, a->content);
        t->next = newnode->attributes;
        newnode->attributes = t;
      }
      break;
    case TEXT_NODE:
      newnode->content = xml_strdup(src->content);
      //fprintf(stderr, "src->content = %s\n", src->content);
      break;
  }
  return newnode;
}

void copy_node_to_result_rec(TRANSFORM_CONTEXT *pctx, XMLNODE *locals, XMLNODE *context, XMLNODE *parent, XMLNODE *src)
{
  XMLNODE *r, *c;
  if(src->type==ATTRIBUTE_NODE) {
    r = xml_new_node(pctx,NULL,ATTRIBUTE_NODE);
    r->name = src->name;
    r->next = parent->attributes;
    parent->attributes = r;
    r->content = src->content;
    src->content = NULL; // to prevent double free
  } else {
    r = copy_node_to_result(pctx,locals,context,parent,src);
    for(c=src->children;c;c=c->next) {
      copy_node_to_result_rec(pctx,locals,context,r,c);
    }
  }
}

char *xml_eval_string(TRANSFORM_CONTEXT *pctx, XMLNODE *locals, XMLNODE *source, XMLNODE *foreval)
{
  XMLNODE *tmp = xml_new_node(pctx,NULL, EMPTY_NODE);
  XMLSTRING res = xmls_new(200);
  int locked = threadpool_lock_on();
  apply_xslt_template(pctx, tmp, source, foreval, NULL, locals);
  if (locked) threadpool_lock_off();
  output_node_rec(tmp, res, pctx);
  xml_free_node(pctx,tmp);
  return xmls_detach(res);
}

void xml_add_attribute(TRANSFORM_CONTEXT *pctx, XMLNODE *parent, char *name, char *value)
{
  XMLNODE *tmp;
  while(parent && parent->type==EMPTY_NODE) {
    parent = parent->parent;
  }
  if(!parent)
    return;

  name = hash(name,-1,0);
  for(tmp = parent->attributes; tmp; tmp=tmp->next) {
    if(tmp->name == name) {
      break;
    }
  }

  if(!tmp) {
    tmp = xml_new_node(pctx,NULL, ATTRIBUTE_NODE);
    tmp->name = name;
    tmp->next = parent->attributes;
    parent->attributes = tmp;
  }
  tmp->content = value;
}  

void apply_xslt_template(TRANSFORM_CONTEXT *pctx, XMLNODE *ret, XMLNODE *source, XMLNODE *templ, XMLNODE *params, XMLNODE *locals)
{
  XMLNODE *instr;
  XMLNODE *tmp;
  XMLNODE *iter;
  XMLNODE *newtempl;

  for (instr = templ; instr; instr = instr->next) 
  {
    debug("apply_xslt_template:: template %s", instr->name);
    if (instr->type == EMPTY_NODE) {
      if (instr->children)
        apply_xslt_template( pctx, tmp, source, instr->children, params, locals );

      continue;
    }

    if (instr->name == NULL || instr->name[0] != 'x') { // hack, speeding up copying of non-xsl elements
      tmp = copy_node_to_result(pctx, locals, source, ret, instr);

      if (instr->children) {
        trace("apply_xslt_template:: starting in thread");
        threadpool_start_full(apply_xslt_template, pctx, tmp, source, instr->children, params, locals);
      }

      continue;
    }

/************************** <xsl:call-template> *****************************/
    if (instr->name == xsl_call) {
      if (newtempl = template_byname(pctx, xml_get_attr(instr,xsl_a_name))) {
        XMLNODE *vars = xml_new_node(pctx, NULL, EMPTY_NODE);
        XMLNODE *param = NULL;

        // process template parameters
        for (iter = instr->children; iter; iter=iter->next) 
        {
          if (iter->name == xsl_withparam) 
          {
            char *pname = hash( xml_get_attr(iter,xsl_a_name),-1,0 );
            char *expr  = xml_get_attr( iter, xsl_a_select );

            // fprintf(stderr, "--------------- %s\npname = %s\nexpr = %s\n", iter->name, pname, expr);

            tmp       = xml_new_node(pctx, pname, ATTRIBUTE_NODE);
            tmp->next = param;
            param     = tmp;

            if (!expr) {
              tmp->extra.v.nodeset = xml_new_node(pctx, NULL, EMPTY_NODE);
              tmp->extra.type      = VAL_NODESET;

              int locked = threadpool_lock_on();
              apply_xslt_template(pctx, tmp->extra.v.nodeset, source, iter->children, NULL, locals);
              if (locked) threadpool_lock_off();
            }
            else {
              xpath_eval_node(pctx, locals, source, expr, &(tmp->extra));
            }
          }
        }

        tmp = xml_append_child(pctx, ret, EMPTY_NODE);
        apply_xslt_template(pctx, tmp, source, newtempl, param, vars);
      }
    }
/************************** <xsl:apply-templates> *****************************/
    else if (instr->name == xsl_apply) 
    {
      char *sval = xml_get_attr(instr,xsl_a_select);
      char *mode = hash(xml_get_attr(instr,xsl_a_mode), -1, 0);

      XMLNODE *tofree = NULL;
      XMLNODE *vars = xml_new_node(pctx, NULL, EMPTY_NODE);
      XMLNODE *child;
      XMLNODE *param = NULL;

      if (sval) {
        if(!instr->compiled) {
          instr->compiled = xpath_find_expr(pctx, sval);
        }
        tofree = iter = xpath_eval_selection(pctx, locals, source, instr->compiled);
      } 
      else {
        iter = source->children;
      }

      // process template parameters
      for (child = instr->children; child; child = child->next) 
      {
        if (child->type == EMPTY_NODE)
          continue;

        if (child->name == xsl_withparam) 
        {
          char *pname = hash(xml_get_attr(child,xsl_a_name),-1,0);
          char *expr  = xml_get_attr(child,xsl_a_select);

          tmp       = xml_new_node(pctx, pname, ATTRIBUTE_NODE);
          tmp->next = param;
          param     = tmp;

          if (!expr) {
            tmp->extra.v.nodeset = xml_new_node(pctx, NULL, EMPTY_NODE);
            tmp->extra.type      = VAL_NODESET;

            int locked = threadpool_lock_on();
            apply_xslt_template(pctx, tmp->extra.v.nodeset, source, child->children, NULL, locals);
            if (locked) threadpool_lock_off();
            tmp = copy_node_to_result(pctx, locals, source, ret, instr);
          } 
          else {
            xpath_eval_node(pctx, locals, source, expr, &(tmp->extra));
          }

          continue;
        }

        if (child->name != xsl_sort)
          break;
        if (!tofree) {
          tofree = iter = xpath_copy_nodeset(iter);
        }
        tofree = iter = xpath_sort_selection(pctx, locals, iter, child);
      }

      for (; iter; iter = iter->next) {
        tmp = xml_append_child(pctx,ret,EMPTY_NODE);
        process_one_node(pctx, tmp, iter, param?param:params, vars, mode);
      }
    }
/************************** <xsl:attribute> *****************************/
    else if(instr->name == xsl_attribute) {
      char *aname = xml_process_string(pctx, locals, source, xml_get_attr(instr,xsl_a_name));
      char *value = xml_eval_string(pctx, locals, source, instr->children);
      char *p;
      for(p=value;*p && x_is_ws(*p);++p)  //remove extra ws at start
          ;
      p=xml_strdup(p);
      xml_add_attribute(pctx,ret,aname,p);
    }
/************************** <xsl:element> *****************************/
    else if(instr->name == xsl_element) {
      char *name = xml_process_string(pctx, locals, source, xml_get_attr(instr,xsl_a_name));
      tmp = xml_append_child(pctx,ret,ELEMENT_NODE);
      tmp->name = hash(name,-1,0);
      name = xml_get_attr(instr,xsl_a_xmlns);
      if(name) {
        name = xml_process_string(pctx, locals, source, name);
        iter = xml_new_node(pctx,xsl_a_xmlns, ATTRIBUTE_NODE);
        iter->content = name;
        tmp->attributes = iter;
      }
      trace("apply_xslt_template:: starting in thread");
      threadpool_start_full(apply_xslt_template,pctx,tmp,source,instr->children,params,locals);
    }
/************************** <xsl:if> *****************************/
    else if(instr->name == xsl_if) {
      if(!instr->compiled) {
        char *expr = xml_get_attr(instr,xsl_a_test);
        instr->compiled = xpath_find_expr(pctx,expr);
      }
      if(xpath_eval_boolean(pctx, locals, source, instr->compiled)) {
        apply_xslt_template(pctx,ret,source,instr->children,params,locals);
      }
    }
/************************** <xsl:choose> *****************************/
    else if(instr->name == xsl_choose) {
      newtempl = NULL;
      for(iter=instr->children;iter;iter=iter->next) {
        if(iter->name==xsl_otherwise)
          newtempl=iter;
        else if(iter->name==xsl_when) {
          if(!iter->compiled) {
            char *expr = xml_get_attr(iter,xsl_a_test);
            iter->compiled = xpath_find_expr(pctx,expr);
          }
          if(xpath_eval_boolean(pctx, locals, source, iter->compiled)) {
            apply_xslt_template(pctx,ret,source,iter->children,params,locals);
            break;
          }
        }
      }
      if(iter==NULL && newtempl) { // no when clause selected -- go for otherwise
        apply_xslt_template(pctx,ret,source,newtempl->children,params,locals);
      }
    }
/************************** <xsl:param> *****************************/
    else if(instr->name == xsl_param) {
      char *pname = hash(xml_get_attr(instr, xsl_a_name), -1, 0);
      char *expr  = xml_get_attr(instr, xsl_a_select);

      XMLNODE n;
      if(!xpath_in_selection(params, pname) && (expr || instr->children)) {
        do_local_var(pctx, locals, source, instr);
      }
      else {
        n.attributes = params;
        add_local_var(pctx, locals, pname, params);
      }
    }
/************************** <xsl:for-each> *****************************/
    else if(instr->name == xsl_foreach) {
      XMLNODE *child;
      if(!instr->compiled) {
        instr->compiled = xpath_find_expr(pctx, xml_get_attr(instr,xsl_a_select));
      }
      iter = xpath_eval_selection(pctx, locals, source, instr->compiled);

      for(child=instr->children;child;child=child->next) {
        if(child->type==EMPTY_NODE)
          continue;
        if(child->name != xsl_sort)
          break;
        iter=xpath_sort_selection(pctx, locals, iter,child);
      }
      tmp = iter;
      for(;iter;iter=iter->next) {
        newtempl = xml_append_child(pctx,ret,EMPTY_NODE);
        trace("apply_xslt_template:: starting in thread");
        threadpool_start_full(apply_xslt_template,pctx,newtempl,iter,child,params,locals);
      }
      xpath_free_selection(pctx,tmp);
    }
/************************** <xsl:copy-of> *****************************/
    else if(instr->name == xsl_copyof) {
      char *sexpr = xml_get_attr(instr,xsl_a_select);
      RVALUE rv;
      xpath_eval_expression(pctx, locals, source, sexpr, &rv);
      if(rv.type != VAL_NODESET) {
        char *ntext = rval2string(&rv);
        if(ntext) {
          newtempl = xml_append_child(pctx,ret,TEXT_NODE);
          newtempl->content = ntext;
        }
      } else {
        for(iter=rv.v.nodeset;iter;iter=iter->next) {
          copy_node_to_result_rec(pctx,locals,source,ret,iter);
        }
        rval_free(&rv);
      }
    }
/************************** <xsl:variable> *****************************/
    else if(instr->name == xsl_var) {
      do_local_var(pctx,locals,source,instr);
    }
/************************** <xsl:value-of> *****************************/
    else if (instr->name == xsl_value_of) 
    {
      if (!instr->compiled)
        instr->compiled = xpath_find_expr(pctx, xml_get_attr(instr, xsl_a_select));

      // fprintf(stderr, "xml_get_attr(instr, xsl_a_select) = %s\n", xml_get_attr(instr, xsl_a_select));

      char *cont = xpath_eval_string(pctx, locals, source, instr->compiled);
      if (cont) 
      {
        tmp          = xml_append_child(pctx, ret, TEXT_NODE);
        tmp->content = cont;
        tmp->flags  |= instr->flags & XML_FLAG_NOESCAPE;
      }
    }
/************************** <xsl:text> *****************************/
    else if(instr->name == xsl_text) {
      char *esc = xml_get_attr(instr,xsl_a_escaping);
      if(esc && esc[0]!='y')
        esc=NULL;
      for(newtempl=instr->children;newtempl;newtempl=newtempl->next) {
        if(newtempl->type==TEXT_NODE) {
          tmp = xml_append_child(pctx,ret,TEXT_NODE);
          tmp->content = xml_strdup(newtempl->content);
          tmp->flags |= instr->flags & XML_FLAG_NOESCAPE;
        }
      }
/************************** <xsl:copy> *****************************/
    } else if(instr->name == xsl_copy) {
      tmp = xml_append_child(pctx,ret,source->type);
      tmp->name = source->name;
      if(instr->children) {
        apply_xslt_template(pctx,tmp,source,instr->children,params,locals);
      }
/************************** <xsl:message> *****************************/
    } else if(instr->name == xsl_message) {
      char *msg = xml_eval_string(pctx, locals, source, instr->children);
      if(msg) {
        debug("apply_xslt_template:: message %s", msg);
      } else {
        error("apply_xslt_template:: fail to get message");
      }
/************************** <xsl:number> ***********************/
    } else if(instr->name == xsl_number) { // currently unused by litres
      tmp = xml_append_child(pctx,ret,TEXT_NODE);
      tmp->content = xml_strdup("1.");
/************************** <xsl:comment> ***********************/
    } else if(instr->name == xsl_comment) {
      tmp = xml_append_child(pctx,ret,COMMENT_NODE);
      tmp->content = xml_eval_string(pctx, locals, source, instr->children);
    }
/************************** <xsl:processing-instruction> ***********************/
    else if(instr->name == xsl_pi) {
      tmp = xml_append_child(pctx,ret,PI_NODE);
      tmp->name = xml_get_attr(instr,xsl_a_name);
      tmp->content = xml_eval_string(pctx, locals, source, instr->children);
    } 
    else if(instr->name[0] == 'x' && instr->name[1] == 's' && instr->name[2] == 'l' && instr->name[3] == ':') {
      error("apply_xslt_template:: XSLT directive <%s> not supported!",instr->name);
      return;
    }
/***************** unknown element - copy to result *************************/
    else {
      tmp = copy_node_to_result(pctx,locals,source,ret,instr);
      if(instr->children) {
        trace("apply_xslt_template:: starting in thread");
        threadpool_start_full(apply_xslt_template,pctx,tmp,source,instr->children,params,locals);
      }
    }
  }
}

void apply_default_process(TRANSFORM_CONTEXT *pctx, XMLNODE *ret, XMLNODE *source, XMLNODE *params, XMLNODE *locals, char *mode)
{
  XMLNODE *child;
  XMLNODE *tmp;

  if(source->type == TEXT_NODE) {
    tmp = xml_append_child(pctx,ret,TEXT_NODE);
    tmp->content = xml_strdup(source->content);
    return;
  }

  for(child=source->children;child;child=child->next) {
    switch(child->type) {
      case TEXT_NODE:
        tmp = xml_append_child(pctx,ret,TEXT_NODE);
        tmp->content = xml_strdup(child->content);
        break;
      default:
        tmp = xml_append_child(pctx,ret,EMPTY_NODE);
        trace("apply_default_process:: starting in thread");
        threadpool_start_full(process_one_node,pctx,tmp,child,params,locals,mode);
    }
  }
}

void process_one_node(TRANSFORM_CONTEXT *pctx, XMLNODE *ret, XMLNODE *source, XMLNODE *params, XMLNODE *locals, char *mode)
{
  XMLNODE *templ;

  if(!source)
    return;
  templ = find_template(pctx, source, mode);

  if(templ) {
    trace("process_one_node:: found template: %s", templ->name);
    // user scope locals for each template
    XMLNODE *scope = xml_new_node(pctx,NULL,EMPTY_NODE);
    apply_xslt_template(pctx, ret, source, templ, params, scope);
  } else {
    apply_default_process(pctx, ret, source, params, locals, mode);
  }
}


char *get_abs_name(TRANSFORM_CONTEXT *pctx, char *fname)
{
  if (!fname) 
    return NULL;
  strcpy(pctx->local_part, fname);
  return pctx->fnbuf;
}


XMLNODE *xsl_preprocess(TRANSFORM_CONTEXT *pctx, XMLNODE *node)
{
  XMLNODE *ret = node;
  XMLNODE *node_next;
  char *pp;

  while(node) {
    node_next = node->next;

    if(node->name == xsl_output && !(pctx->flags & XSL_FLAG_OUTPUT)) {
      pctx->flags |= XSL_FLAG_OUTPUT;
      pctx->doctype_public = xml_get_attr(node,xsl_a_dtpublic);
      pctx->doctype_system = xml_get_attr(node,xsl_a_dtsystem);
      pctx->encoding = xml_get_attr(node,xsl_a_encoding);
      pctx->media_type = xml_get_attr(node,xsl_a_media);
      pp = xml_get_attr(node,xsl_a_method);
      if(pp) {
        if(0==xml_strcasecmp(pp,"xml"))
          pctx->outmode = MODE_XML;
        else if(0==xml_strcasecmp(pp,"html"))
          pctx->outmode = MODE_HTML;
        else if(0==xml_strcasecmp(pp,"text"))
          pctx->outmode = MODE_TEXT;
        pctx->flags|=XSL_FLAG_MODE_SET;
      }

      pp = xml_get_attr(node,xsl_a_omitxml);
      if(0==xml_strcasecmp(pp,"yes"))
        pctx->flags |= XSL_FLAG_OMIT_DESC;

      pp = xml_get_attr(node,xsl_a_standalone);
      if(0==xml_strcasecmp(pp,"yes"))
        pctx->flags |= XSL_FLAG_STANDALONE;

    } else if(node->name == xsl_include) {
      char *name = get_abs_name(pctx, xml_get_attr(node,xsl_a_href));
      XMLNODE *included;
      if(name) {
        trace("xsl_preprocess:: include file: %s", name);
        included = xml_parse_file(pctx->gctx, name, 1);
        if(included) {
          included->parent = node;
          node->type=EMPTY_NODE;
          node->children=included;
        } else {
          error("xsl_preprocess:: failed to include %s",pctx->fnbuf);
        }
      }
    }

    if(node->type == TEXT_NODE && node->parent->name != xsl_text) {
      char *p = node->content;
      for(;*p;++p) {
        if(!x_is_ws(*p))
          break;
      }
      if(*p==0) {
        node->content = NULL;
        xml_unlink_node(node);
        nfree(pctx,node);
        if(node==ret)
          ret = node_next;
        node = node_next;
        continue;
      }
    }

    if(node->children) {
    	// fprintf(stderr, "xsl_preprocess: node->children exists\n");
      char *save_local = pctx->local_part;
      char *s = strrchr(pctx->local_part,'/');
      if(s)
        pctx->local_part = s+1;
      node->children = xsl_preprocess(pctx, node->children);
      pctx->local_part = save_local;
    }
    node = node_next;
  }
  return ret;
}


void process_imports(TRANSFORM_CONTEXT *pctx, XMLNODE *node)
{
  for(;node;node=node->next) 
  {
    if(node->name == xsl_import) 
    {
      char *name = get_abs_name(pctx, xml_get_attr(node,xsl_a_href));
      if(name) 
      {
        char *save_local = pctx->local_part;
        char *s = strrchr(pctx->local_part, '/');
        if (s) pctx->local_part = s + 1;

       	debug("process_imports:: import %s", name);
        XMLNODE *included = xml_parse_file(pctx->gctx, name, 1);
        if(included != NULL) 
        {
          xsl_preprocess(pctx, included);  //process includes and clean-up
          process_imports(pctx, included); //process imports if any
          node->type = EMPTY_NODE;
          node->children = included;
        }
        else {
          debug("process_imports:: failed to import %s", pctx->fnbuf);
        }
        pctx->local_part = save_local;
      }
    }
    else if(node->children) {
      trace("process_imports:: process children");
      process_imports(pctx,node->children); //process imports if any
    }
  }
}

void preformat_document(TRANSFORM_CONTEXT *pctx, XMLNODE *doc)
{
    // TODO apply strip-space
}

void process_global_flags(TRANSFORM_CONTEXT *pctx, XMLNODE *node)
{
  char *name;
  XMLNODE *tmp;

  while(node) {
    if(node->name == xsl_text||node->name == xsl_value_of) { // disable escaping for text and value-of
      char *dis = xml_get_attr(node,xsl_a_escaping);
      if(0==xml_strcasecmp(dis,"yes")) {
        node->flags |= XML_FLAG_NOESCAPE;
      }
    }
/*********** process xsl:key instructions ************/
    if(node->name==xsl_key) {
      name = xml_get_attr(node, xsl_a_name);
      tmp = xml_new_node(pctx, name, KEY_NODE);

      char *match = xml_get_attr(node, xsl_a_match);
      char *use = xml_get_attr(node, xsl_a_use);
      char *format = "%s[%s = '%%s']";
      int size = snprintf(NULL, 0, format, match, use);
      if (size > 0) {
        int buffer_size = size + 1;
        char *buffer = memory_allocator_new(buffer_size);
        if (snprintf(buffer, buffer_size, format, match, use) == size) {
          debug("process_global_flags:: key predicate: %s", buffer);
          tmp->content = buffer;
        }
      }

      tmp->next = pctx->keys;
      pctx->keys = tmp;
    }
/*********** process xsl:sort instructions ************/
    else if(node->name==xsl_sort) {
      node->compiled = xpath_find_expr(pctx,xml_get_attr(node, xsl_a_select));
      name = xml_get_attr(node, xsl_a_datatype);
      if(0==xml_strcasecmp(name, "number"))
        node->flags |= XML_FLAG_SORTNUMBER;
      name = xml_get_attr(node, xsl_a_order);
      if(0==xml_strcasecmp(name, "descending"))
        node->flags |= XML_FLAG_DESCENDING;
      name = xml_get_attr(node, xsl_a_caseorder);
      if(0==xml_strcasecmp(name, "lower-first"))
        node->flags |= XML_FLAG_LOWER;
    }
/*********** process xsl:decimal-format instructions ************/
    else if(node->name==xsl_decimal) {
      name = xml_get_attr(node, xsl_a_name);
      tmp = xml_new_node(pctx, name, ATTRIBUTE_NODE);
      tmp->children = node;
      tmp->next = pctx->formats;
      pctx->formats = tmp;
    }
    if(node->children)
      process_global_flags(pctx, node->children);
    node = node->next;
  }
}

/************************** Interface functions ******************************/

void XSLTEnd(XSLTGLOBALDATA *data)
{
  info("XSLTEnd");
  drop_hash();
  dict_free(data->urldict);
  if (data->cache != NULL) external_cache_release(data->cache);

  free(data->perl_functions);
  free(data);

  logger_release();
}

XSLTGLOBALDATA *XSLTInit(void *interpreter)
{
  logger_create();

  info("XSLTInit");
  XSLTGLOBALDATA *ret = malloc(sizeof(XSLTGLOBALDATA));
  memset(ret,0,sizeof(XSLTGLOBALDATA));
  init_processor(ret);
  ret->urldict = dict_new(300);
  ret->interpreter = interpreter;

  return ret;
}

void XSLTEnableExternalCache(XSLTGLOBALDATA *data, char *server_list)
{
  data->cache = external_cache_create(server_list);
}

void xml_free_document(XMLNODE *doc)
{
  debug("xml_free_document:: file %s", doc->file);

  XMLNODE *children = doc->children;
  if (children != NULL) xml_free_document(children);

  if (doc->allocator != NULL) memory_allocator_release(doc->allocator);
}

void XMLFreeDocument(XMLNODE *doc)
{
  info("XMLFreeDocument:: file: %s", doc->file);
  xml_free_document(doc);
  hash_memory_usage();
}

void XSLTFreeProcessor(TRANSFORM_CONTEXT *pctx)
{
  info("XSLTFreeProcessor");
  dict_free(pctx->named_templ);
  if(pctx->keys)
    xml_free_node(NULL,pctx->keys);
  if(pctx->formats)
    xml_free_node(NULL,pctx->formats);
  xpath_free_compiled(pctx);
  xml_free_document(pctx->stylesheet);

  memory_allocator_release(pctx->allocator);
  threadpool_free(pctx->pool);

  free(pctx->templtab);
  free(pctx->sort_keys);
  free(pctx->sort_nodes);
  free(pctx->functions);
  free(pctx->fnbuf);
  free(pctx);
}

TRANSFORM_CONTEXT *XSLTNewProcessor(XSLTGLOBALDATA *gctx, char *stylesheet)
{
  info("XSLTNewProcessor:: stylesheet %s", stylesheet);

  TRANSFORM_CONTEXT *ret = malloc(sizeof(TRANSFORM_CONTEXT));
  if(!ret) {
    error("XSLTNewProcessor:: create context");
    return NULL;
  }
  memset(ret,0,sizeof(TRANSFORM_CONTEXT));

  pthread_mutexattr_t attribute;
  if (pthread_mutexattr_init(&attribute)) {
    error("XSLTNewProcessor:: create lock");
    return NULL;
  }
  pthread_mutexattr_settype(&attribute, PTHREAD_MUTEX_RECURSIVE);
  if(pthread_mutex_init(&(ret->lock), &attribute)) {
    error("XSLTNewProcessor:: create lock");
    return NULL;
  }
  pthread_mutexattr_destroy(&attribute);

  ret->allocator = memory_allocator_create();
  if (ret->allocator == NULL)
  {
      return NULL;
  }
  memory_allocator_add_entry(ret->allocator, pthread_self(), 10000000);
  memory_allocator_set_current(ret->allocator);

  ret->gctx = gctx;
  ret->stylesheet = XMLParseFile(gctx, stylesheet);
  if(!ret->stylesheet) {
    free(ret);
    return NULL;
  }

  ret->outmode = MODE_XML;
  ret->fnbuf = malloc(1024); // XXX may be not enough for pathname but passable
  strcpy(ret->fnbuf, stylesheet);
  ret->local_part = strrchr(ret->fnbuf,'/');
  if(ret->local_part)
    ++ret->local_part; //after last /
  else
    ret->local_part = ret->fnbuf; // no path in filename

  ret->sort_size = 100;
  ret->sort_keys = (char **)malloc(ret->sort_size*sizeof(char *));
  ret->sort_nodes = (XMLNODE **)malloc(ret->sort_size*sizeof(XMLNODE *));

  ret->m_exprs = 200;
  ret->compiled = malloc(ret->m_exprs*sizeof(EXPTAB));

  ret->named_templ = dict_new(300);

  ret->stylesheet = xsl_preprocess(ret, ret->stylesheet);  //root node is always empty
  process_imports(ret, ret->stylesheet->children);
  precompile_templates(ret, ret->stylesheet);
  process_global_flags(ret, ret->stylesheet);
  return ret;
}

XMLNODE *find_first_node(XMLNODE *n)
{
  XMLNODE *t;

  for(;n;n=n->next) {
    if(n->type != EMPTY_NODE)
      return n;
    if(t=find_first_node(n->children))
      return t;
  }
  return NULL;
}

void XSLTCreateThreadPool(TRANSFORM_CONTEXT *pctx, unsigned int size)
{
  if (pctx->pool != NULL)
  {
    error("XSLTCreateThreadPool:: thread pool exists");
    return;
  }

  pctx->pool = threadpool_init(size);
  if (pctx->gctx->cache != NULL) threadpool_set_external_cache(pctx->gctx->cache, pctx->pool);
}

XMLNODE *XSLTProcess(TRANSFORM_CONTEXT *pctx, XMLNODE *document)
{
  if(!pctx) {
    error("XSLTProcess:: pctx is NULL");
    return NULL;
  }

  if(!document) {
    error("XSLTProcess:: document is NULL");
    return NULL;
  }

  // memory cache for output document
  memory_allocator *allocator = memory_allocator_create();
  memory_allocator_set_parent(allocator, pctx->allocator);
  memory_allocator_add_entry(allocator, pthread_self(), 10000000);
  memory_allocator_set_current(allocator);

  XMLNODE *locals = xml_new_node(pctx,NULL,EMPTY_NODE);
  XMLNODE *ret = xml_new_node(pctx, "out",EMPTY_NODE);
  ret->allocator = allocator;

  pctx->root_node = document;
  precompile_variables(pctx, pctx->stylesheet->children, document);
  preformat_document(pctx,document);

  threadpool_set_allocator(allocator, pctx->pool);

  info("XSLTProcess:: start process");
  process_one_node(pctx, ret, document, NULL, locals, NULL);
  threadpool_wait(pctx->pool);
  info("XSLTProcess:: end process");

/****************** add dtd et al if required *******************/
  XMLNODE *t = find_first_node(ret);

  if(pctx->outmode==MODE_XML && !(pctx->flags&XSL_FLAG_MODE_SET)) {
    pctx->outmode=MODE_XML;
    if(t->type==ELEMENT_NODE) {
      if(strcasestr(t->name,"html"))
          pctx->outmode=MODE_HTML;
    }
  }

  if(pctx->outmode==MODE_HTML) {
    XMLNODE *sel;
    sel = xpath_eval_selection(pctx, locals, t, xpath_find_expr(pctx,"head"));
    if(sel) {
      XMLNODE *meta = xml_new_node(pctx, NULL,TEXT_NODE);
      meta->content = xml_strdup("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">");
      meta->flags = XML_FLAG_NOESCAPE;
      meta->next = sel->children;
      if(sel->children) {
        sel->children->prev = meta;
        meta->parent = sel->children->parent;
        if(sel->children->parent)
          sel->children->parent->children = meta;
      }
      xpath_free_selection(pctx,sel);
    }
  }

  xml_free_node(pctx,locals);
  free_variables(pctx);

  return ret;
}
