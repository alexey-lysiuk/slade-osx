/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_sizer.h
// Purpose:     XML resource handler for wxBoxSizer
// Author:      Vaclav Slavik
// Created:     2000/04/24
// RCS-ID:      $Id: xh_sizer.h 72710 2012-10-19 22:03:25Z VZ $
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_SIZER_H_
#define _WX_XH_SIZER_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC

#include "wx/sizer.h"
#include "wx/gbsizer.h"

class WXDLLIMPEXP_XRC wxSizerXmlHandler : public wxXmlResourceHandler
{
    DECLARE_DYNAMIC_CLASS(wxSizerXmlHandler)

public:
    wxSizerXmlHandler();
    virtual wxObject *DoCreateResource();
    virtual bool CanHandle(wxXmlNode *node);

protected:
    virtual wxSizer* DoCreateSizer(const wxString& name);
    virtual bool IsSizerNode(wxXmlNode *node) const;

private:
    bool m_isInside;
    bool m_isGBS;

    wxSizer *m_parentSizer;


    wxObject* Handle_sizeritem();
    wxObject* Handle_spacer();
    wxObject* Handle_sizer();
    wxSizer*  Handle_wxBoxSizer();
#if wxUSE_STATBOX
    wxSizer*  Handle_wxStaticBoxSizer();
#endif
    wxSizer*  Handle_wxGridSizer();
    wxFlexGridSizer* Handle_wxFlexGridSizer();
    wxGridBagSizer* Handle_wxGridBagSizer();
    wxSizer*  Handle_wxWrapSizer();

    bool ValidateGridSizerChildren();
    void SetFlexibleMode(wxFlexGridSizer* fsizer);
    void SetGrowables(wxFlexGridSizer* fsizer, const wxChar* param, bool rows);
    wxGBPosition GetGBPos(const wxString& param);
    wxGBSpan GetGBSpan(const wxString& param);
    wxSizerItem* MakeSizerItem();
    void SetSizerItemAttributes(wxSizerItem* sitem);
    void AddSizerItem(wxSizerItem* sitem);
};

#if wxUSE_BUTTON

class WXDLLIMPEXP_XRC wxStdDialogButtonSizerXmlHandler
    : public wxXmlResourceHandler
{
    DECLARE_DYNAMIC_CLASS(wxStdDialogButtonSizerXmlHandler)

public:
    wxStdDialogButtonSizerXmlHandler();
    virtual wxObject *DoCreateResource();
    virtual bool CanHandle(wxXmlNode *node);

private:
    bool m_isInside;
    wxStdDialogButtonSizer *m_parentSizer;
};

#endif // wxUSE_BUTTON

#endif // wxUSE_XRC

#endif // _WX_XH_SIZER_H_
