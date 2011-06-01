//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#include "lomse_linker.h"

#include "lomse_internal_model.h"
#include "lomse_ldp_elements.h"        //for node type
#include "lomse_im_note.h"


using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
ImoObj* Linker::add_child_to_model(ImoObj* pParent, ImoObj* pChild, int ldpChildType)
{
    //returns the object for set_imo() method (can be NULL)

    m_ldpChildType = ldpChildType;
    m_pParent = pParent;

    switch(pChild->get_obj_type())
    {
        case ImoObj::k_bezier_info:
            return add_bezier(dynamic_cast<ImoBezierInfo*>(pChild));

        case ImoObj::k_content:
            return add_child(ImoObj::k_document, pChild);

        case ImoObj::k_cursor_info:
            return add_cursor(dynamic_cast<ImoCursorInfo*>(pChild));

        case ImoObj::k_font_info:
            return add_font_info(dynamic_cast<ImoFontInfo*>(pChild));

        case ImoObj::k_instrument:
            return add_instrument(dynamic_cast<ImoInstrument*>(pChild));

        case ImoObj::k_instr_group:
            return add_instruments_group(dynamic_cast<ImoInstrGroup*>(pChild));

        case ImoObj::k_midi_info:
            return add_midi_info(dynamic_cast<ImoMidiInfo*>(pChild));

        case ImoObj::k_music_data:
            return add_child(ImoObj::k_instrument, pChild);

        case ImoObj::k_option:
            return add_option(dynamic_cast<ImoOptionInfo*>(pChild));

        case ImoObj::k_page_info:
            return add_page_info(dynamic_cast<ImoPageInfo*>(pChild));

        case ImoObj::k_score_text:
            return add_text(dynamic_cast<ImoScoreText*>(pChild));

        case ImoObj::k_score_title:
            return add_title(dynamic_cast<ImoScoreTitle*>(pChild));

        case ImoObj::k_staff_info:
            return add_staff_info(dynamic_cast<ImoStaffInfo*>(pChild));

        case ImoObj::k_styles:
            return add_child(ImoObj::k_document, pChild);

        case ImoObj::k_system_info:
            return add_system_info(dynamic_cast<ImoSystemInfo*>(pChild));

        case ImoObj::k_text_item:
            return add_text_item(dynamic_cast<ImoTextItem*>(pChild));

        case ImoObj::k_text_style_info:
            return add_text_style_info(dynamic_cast<ImoTextStyleInfo*>(pChild));

        default:
            if (pChild->is_boxobj())
                return add_child(ImoObj::k_content, pChild);
            else if (pChild->is_staffobj())
                return add_staffobj(dynamic_cast<ImoStaffObj*>(pChild));
            else if (pChild->is_auxobj())
                return add_attachment(dynamic_cast<ImoAuxObj*>(pChild));
            else
                return pChild;
    }
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_instruments_group(ImoInstrGroup* pGrp)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = dynamic_cast<ImoScore*>(m_pParent);
        pScore->add_instruments_group(pGrp);
    }
    return pGrp;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_option(ImoOptionInfo* pOpt)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = dynamic_cast<ImoScore*>( m_pParent );
        pScore->set_option(pOpt);
        delete pOpt;
        return NULL;
    }
    return pOpt;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_page_info(ImoPageInfo* pPI)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = dynamic_cast<ImoScore*>(m_pParent);
        pScore->add_page_info(pPI);
        delete pPI;
        return NULL;
    }
    else if (m_pParent && m_pParent->is_document())
    {
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(m_pParent);
        pDoc->add_page_info(pPI);
        delete pPI;
        return NULL;
    }
    return pPI;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_cursor(ImoCursorInfo* pCursor)
{
    if (m_pParent && m_pParent->is_document())
    {
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(m_pParent);
        pDoc->add_cursor_info(pCursor);
        delete pCursor;
        return NULL;
    }
    return pCursor;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_system_info(ImoSystemInfo* pSI)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = dynamic_cast<ImoScore*>(m_pParent);
        pScore->add_sytem_info(pSI);
        delete pSI;
        return NULL;
    }
    return pSI;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_text_style_info(ImoTextStyleInfo* pStyle)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = dynamic_cast<ImoScore*>(m_pParent);
        pScore->add_style_info(pStyle);
        return NULL;
    }
    else if (m_pParent && m_pParent->is_styles())
    {
        ImoStyles* pStyles = dynamic_cast<ImoStyles*>(m_pParent);
        pStyles->add_style_info(pStyle);
        return NULL;
    }
    return pStyle;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_font_info(ImoFontInfo* pFont)
{
    if (m_pParent && m_pParent->is_text_style_info())
    {
        ImoTextStyleInfo* pStyle = dynamic_cast<ImoTextStyleInfo*>(m_pParent);
        pStyle->set_font_size(pFont->size);
        pStyle->set_font_weight(pFont->weight);
        pStyle->set_font_style(pFont->style);
        pStyle->set_font_name(pFont->name);
        delete pFont;
        return NULL;
    }
    return pFont;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_bezier(ImoBezierInfo* pBezier)
{
    if (m_pParent && m_pParent->is_tie_dto())
    {
        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>(m_pParent);
        pInfo->set_bezier(pBezier);
        return NULL;
    }
    else if (m_pParent && m_pParent->is_slur_dto())
    {
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>(m_pParent);
        pInfo->set_bezier(pBezier);
        return NULL;
    }
    return pBezier;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_midi_info(ImoMidiInfo* pInfo)
{
    if (m_pParent && m_pParent->is_instrument())
    {
        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>(m_pParent);
        pInstr->set_midi_info(pInfo);
        return NULL;
    }
    return pInfo;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_staff_info(ImoStaffInfo* pInfo)
{
    if (m_pParent && m_pParent->is_instrument())
    {
        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>(m_pParent);
        pInstr->replace_staff_info(pInfo);
        return NULL;
    }
    return pInfo;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_instrument(ImoInstrument* pInstrument)
{
    if (m_pParent)
    {
        if (m_pParent->is_instr_group())
        {
            ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( m_pParent );
            pGrp->add_instrument(pInstrument);
        }
        else if (m_pParent->is_score())
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( m_pParent );
            pScore->add_instrument(pInstrument);
        }
    }
    return pInstrument;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_text(ImoScoreText* pText)
{
    if (m_pParent)
    {
        //compatibility with 1.5. Since 1.6 auxObjs can not be included
        //in musicData; they must go attached to an spacer.
        if (m_pParent->is_music_data())
        {
            //musicData: create anchor (ImoSpacer) and attach to it
            ImoSpacer* pSpacer = new ImoSpacer();
            pSpacer->add_attachment(pText);
            add_staffobj(pSpacer);
            return pText;
        }

        if (m_pParent->is_instrument())
        {
            ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>(m_pParent);
            //could be 'name' or 'abbrev'
            if (m_ldpChildType == k_name)
                pInstr->set_name(pText);
            else
                pInstr->set_abbrev(pText);
            return NULL;
        }

        if (m_pParent->is_instr_group())
        {
            ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>(m_pParent);
            //could be 'name' or 'abbrev'
            if (m_ldpChildType == k_name)
                pGrp->set_name(pText);
            else
                pGrp->set_abbrev(pText);
            return NULL;
        }

        if (m_pParent->is_content())
        {
            add_child(ImoObj::k_content, pText);
            return pText;
        }

        return add_attachment(pText);
    }
    return pText;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_text_item(ImoTextItem* pText)
{
    if (m_pParent)
    {
        if (m_pParent->is_textblock())
        {
            ImoTextBlock* pBox = dynamic_cast<ImoTextBlock*>(m_pParent);
            pBox->add_item(pText);
            return NULL;
        }

        if (m_pParent->is_content())
        {
            add_child(ImoObj::k_content, pText);
            return NULL;
        }
    }
    return pText;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_title(ImoScoreTitle* pTitle)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = dynamic_cast<ImoScore*>(m_pParent);
        pScore->add_title(pTitle);
    }
    return pTitle;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_child(int parentType, ImoObj* pImo)
{
    if (m_pParent && m_pParent->get_obj_type() == parentType)
        m_pParent->append_child(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_staffobj(ImoStaffObj* pSO)
{
    if (m_pParent)
    {
        if (m_pParent->is_music_data())
            m_pParent->append_child(pSO);
        else if (m_pParent->is_chord() && pSO->is_note())
        {
            ImoChord* pChord = dynamic_cast<ImoChord*>(m_pParent);
            ImoNote* pNote = dynamic_cast<ImoNote*>(pSO);
            pNote->include_in_relation(pChord);
            return NULL;
        }
    }
    return pSO;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_attachment(ImoAuxObj* pAuxObj)
{
    if (m_pParent && m_pParent->is_contentobj())
    {
        ImoContentObj* pContentObj = dynamic_cast<ImoContentObj*>(m_pParent);
        pContentObj->add_attachment(pAuxObj);
    }
    return pAuxObj;
}



}   //namespace lomse
