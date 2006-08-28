/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************

This software module was originally developed by

Heiko Schwarz    (Fraunhofer HHI),
Tobias Hinz      (Fraunhofer HHI),
Karsten Suehring (Fraunhofer HHI)

in the course of development of the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video
Coding) for reference purposes and its performance may not have been optimized.
This software module is an implementation of one or more tools as specified by
the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding).

Those intending to use this software module in products are advised that its
use may infringe existing patents. ISO/IEC have no liability for use of this
software module or modifications thereof.

Assurance that the originally developed software module can be used
(1) in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) once the
ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) has been adopted; and
(2) to develop the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding): 

To the extent that Fraunhofer HHI owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, Fraunhofer HHI will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

Fraunhofer HHI retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards. 

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005. 

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The Fraunhofer HHI hereby donate this source code to the ITU, with the following
understanding:
    1. Fraunhofer HHI retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. Fraunhofer HHI retain full patent rights (if any exist) in the technical
       content of techniques and algorithms herein.
    3. The ITU shall make this code available to anyone, free of license or
       royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from 
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/



#if !defined(AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_)
#define AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "H264AVCCommonLib/CommonBuffers.h"
#include "H264AVCCommonLib/HeaderSymbolReadIf.h"
#include "H264AVCCommonLib/HeaderSymbolWriteIf.h"
#include <list>

#define MAX_NUM_LAYER 6



H264AVC_NAMESPACE_BEGIN



class ParameterSetMng;


#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API SEI
{

public:
  enum MessageType
  {
    SUB_SEQ_INFO                          = 10,
	MOTION_SEI                            = 18,
    SCALABLE_SEI                          = 22,
		SUB_PIC_SEI														= 23,
    //{{Quality level estimation and modified truncation- JVTO044 and m12007
    //France Telecom R&D-(nathalie.cammas@francetelecom.com)
    QUALITYLEVEL_SEI                      = 25,
	//}}Quality level estimation and modified truncation- JVTO044 and m12007
	// JVT-S080 LMI {
	SCALABLE_SEI_LAYERS_NOT_PRESENT       = 26,
    SCALABLE_SEI_DEPENDENCY_CHANGE        = 27,
    // JVT-T073    RESERVED_SEI                          = 28,
	// JVT-S080 LMI }
    //  JVT-T073 {
	SCENE_INFO_SEI                        = 9,
	SCALABLE_NESTING_SEI                  = 28,   
	RESERVED_SEI                          = 29,
    //  JVT-T073 }
  	NON_REQUIRED_SEI					            = 24
  };


  class H264AVCCOMMONLIB_API SEIMessage
  {
  public:
    virtual ~SEIMessage()                                                       {}
    MessageType     getMessageType()                                      const { return m_eMessageType; }
    virtual ErrVal  write         ( HeaderSymbolWriteIf* pcWriteIf ) = 0;
    virtual ErrVal  read          ( HeaderSymbolReadIf*   pcReadIf ) = 0;

  protected:
    SEIMessage( MessageType eMessageType) : m_eMessageType( eMessageType ) {}

  private:
    MessageType m_eMessageType;
  };



  class H264AVCCOMMONLIB_API ReservedSei : public SEIMessage
  {
  protected:
    ReservedSei( UInt uiSize = 0 ) : SEIMessage(RESERVED_SEI), m_uiSize(uiSize) {}

  public:
    static ErrVal create( ReservedSei*&         rpcReservedSei,
                          UInt                  uiSize );
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
  
  private:
    UInt m_uiSize;
  };


  class H264AVCCOMMONLIB_API SubSeqInfo : public SEIMessage
  {
  protected:
    SubSeqInfo()
      : SEIMessage(SUB_SEQ_INFO)
      , m_uiSubSeqLayerNum      (0)
	    , m_uiSubSeqId            (0)
	    , m_bFirstRefPicFlag      (false)
	    , m_bLeadingNonRefPicFlag (false)
	    , m_bLastPicFlag          (false)
	    , m_bSubSeqFrameNumFlag   (false)
      , m_uiSubSeqFrameNum      (0)
    {}

  public:
    static ErrVal create( SubSeqInfo*&          rpcSEIMessage );
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        init  ( UInt                  uiSubSeqLayerNum,
	                        UInt                  uiSubSeqId,
	                        Bool                  bFirstRefPicFlag,
	                        Bool                  bLeadingNonRefPicFlag,
	                        Bool                  bLastPicFlag        = false,
	                        Bool                  bSubSeqFrameNumFlag = false,
                          UInt                  uiSubSeqFrameNum    = 0 );

    UInt getSubSeqId      ()  const { return m_uiSubSeqId; }
    UInt getSubSeqLayerNum()  const { return m_uiSubSeqLayerNum; }

  private:
	  UInt  m_uiSubSeqLayerNum;
	  UInt  m_uiSubSeqId;
	  Bool  m_bFirstRefPicFlag;
	  Bool  m_bLeadingNonRefPicFlag;
	  Bool  m_bLastPicFlag;
	  Bool  m_bSubSeqFrameNumFlag;
    UInt  m_uiSubSeqFrameNum;
  };

	class H264AVCCOMMONLIB_API ScalableSei: public SEIMessage
	{
	protected:
		ScalableSei ();
		~ScalableSei();

	public:
		static ErrVal create ( ScalableSei*&			rpcSeiMessage);
		ErrVal write				 ( HeaderSymbolWriteIf	*pcWriteIf);
		ErrVal read					 ( HeaderSymbolReadIf		*pcReadIf);

		Void setNumLayersMinus1( UInt ui )																				{ m_num_layers_minus1 = ui;	}
		Void setLayerId ( UInt uilayer, UInt uiId )																{ m_layer_id															[uilayer] = uiId; }
	//JVT-S036 lsj start
//		Void setFGSlayerFlag ( UInt uilayer, Bool bFlag )													{ m_fgs_layer_flag												[uilayer] = bFlag; }   
		Void setSimplePriorityId ( UInt uilayer, UInt uiLevel )										{ m_simple_priority_id										[uilayer] = uiLevel; }
		Void setDiscardableFlag	(UInt uilayer, Bool bFlag)												{ m_discardable_flag											[uilayer] = bFlag; }
		Void setTemporalLevel ( UInt uilayer, UInt uiLevel )											{ m_temporal_level												[uilayer] = uiLevel; }
		Void setDependencyId ( UInt uilayer, UInt uiId )													{ m_dependency_id													[uilayer] = uiId; }
		Void setQualityLevel ( UInt uilayer, UInt uiLevel )												{ m_quality_level													[uilayer] = uiLevel; }
	
		Void setSubPicLayerFlag ( UInt uilayer, Bool bFlag)												{ m_sub_pic_layer_flag[uilayer] = bFlag; }
		Void setSubRegionLayerFlag ( UInt uilayer, Bool bFlag)										{ m_sub_region_layer_flag									[uilayer] = bFlag; }
		Void setIroiSliceDivisionInfoPresentFlag ( UInt uilayer, Bool bFlag )				{ m_iroi_slice_division_info_present_flag		[uilayer] = bFlag; } 
		Void setProfileLevelInfoPresentFlag ( UInt uilayer, Bool bFlag)						{ m_profile_level_info_present_flag				[uilayer] = bFlag; }
	//JVT-S036 lsj end

		Void setBitrateInfoPresentFlag ( UInt uilayer, Bool bFlag )								{ m_bitrate_info_present_flag							[uilayer] = bFlag; }
		Void setFrmRateInfoPresentFlag ( UInt uilayer, Bool bFlag )								{ m_frm_rate_info_present_flag						[uilayer] = bFlag; }
		Void setFrmSizeInfoPresentFlag ( UInt uilayer, Bool bFlag )								{ m_frm_size_info_present_flag						[uilayer] = bFlag; }
		Void setLayerDependencyInfoPresentFlag ( UInt uilayer, Bool bFlag )				{ m_layer_dependency_info_present_flag		[uilayer] = bFlag; }
		Void setInitParameterSetsInfoPresentFlag ( UInt uilayer, Bool bFlag )			{ m_init_parameter_sets_info_present_flag	[uilayer] = bFlag; }
		Void setExactInterlayerPredFlag ( UInt uilayer, Bool bFlag )			{ m_exact_interlayer_pred_flag  [uilayer] = bFlag; }				//JVT-S036 lsj
		Void setLayerProfileIdc ( UInt uilayer, UInt uiIdc )											{ m_layer_profile_idc											[uilayer] = uiIdc; }
		Void setLayerConstraintSet0Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set0_flag						[uilayer] = bFlag; }
		Void setLayerConstraintSet1Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set1_flag						[uilayer] = bFlag; }
		Void setLayerConstraintSet2Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set2_flag						[uilayer] = bFlag; }
		Void setLayerConstraintSet3Flag ( UInt uilayer, Bool bFlag )							{ m_layer_constraint_set3_flag						[uilayer] = bFlag; }
		Void setLayerLevelIdc ( UInt uilayer, UInt uiIdc )												{ m_layer_level_idc												[uilayer] = uiIdc; }
		
	//JVT-S036 lsj start
		Void setProfileLevelInfoSrcLayerIdDelta ( UInt uilayer, UInt uiIdc ) { m_profile_level_info_src_layer_id_delta [uilayer] = uiIdc; } 
	
		Void setAvgBitrate ( UInt uilayer, UInt uiBitrate )												{ m_avg_bitrate										[uilayer] = uiBitrate; }
		Void setMaxBitrateLayer ( UInt uilayer, UInt uiBitrate )										{ m_max_bitrate_layer								[uilayer] = uiBitrate; }
		Void setMaxBitrateDecodedPicture ( UInt uilayer, UInt uiBitrate )								{ m_max_bitrate_decoded_picture						[uilayer] = uiBitrate; }		
		Void setMaxBitrateCalcWindow ( UInt uilayer, UInt uiBitrate )									{ m_max_bitrate_calc_window							[uilayer] = uiBitrate; }
	//JVT-S036 lsj end
		
		
		Void setConstantFrmRateIdc ( UInt uilayer, UInt uiFrmrate )								{ m_constant_frm_rate_idc									[uilayer] = uiFrmrate; }
		Void setAvgFrmRate ( UInt uilayer, UInt uiFrmrate )												{ m_avg_frm_rate													[uilayer] = uiFrmrate; }
		Void setFrmRateInfoSrcLayerIdDelta( UInt uilayer, UInt uiFrmrate)					{ m_frm_rate_info_src_layer_id_delta			[uilayer] = uiFrmrate; } //JVT-S036 lsj
		Void setFrmWidthInMbsMinus1 ( UInt uilayer, UInt uiWidth )								{ m_frm_width_in_mbs_minus1								[uilayer] = uiWidth; }
		Void setFrmHeightInMbsMinus1 ( UInt uilayer, UInt uiHeight )							{ m_frm_height_in_mbs_minus1							[uilayer] = uiHeight; }
		Void setFrmSizeInfoSrcLayerIdDelta ( UInt uilayer, UInt uiFrmsize)					{ m_frm_size_info_src_layer_id_delta			[uilayer] = uiFrmsize; } //JVT-S036 lsj
		Void setBaseRegionLayerId ( UInt uilayer, UInt uiId )											{ m_base_region_layer_id									[uilayer] = uiId; }
		Void setDynamicRectFlag ( UInt uilayer, Bool bFlag )											{ m_dynamic_rect_flag											[uilayer] = bFlag; }
		Void setHorizontalOffset ( UInt uilayer, UInt uiOffset )									{ m_horizontal_offset											[uilayer] = uiOffset; }
		Void setVerticalOffset ( UInt uilayer, UInt uiOffset )										{ m_vertical_offset												[uilayer] = uiOffset; }
		Void setRegionWidth ( UInt uilayer, UInt uiWidth )												{ m_region_width													[uilayer] = uiWidth; }
		Void setRegionHeight ( UInt uilayer, UInt uiHeight )											{ m_region_height													[uilayer] = uiHeight; }
		Void setSubRegionInfoSrcLayerIdDelta ( UInt uilayer, UInt uiSubRegion )					{ m_sub_region_info_src_layer_id_delta						[uilayer] = uiSubRegion; } //JVT-S036 lsj
	//JVT-S036 lsj start
		Void setRoiId ( UInt uilayer, UInt RoiId )												{ m_roi_id[uilayer]	= RoiId; } 
		Void setIroiSliceDivisionType ( UInt uilayer, UInt bType )								{ m_iroi_slice_division_type[uilayer] = bType; }
		Void setGridSliceWidthInMbsMinus1 ( UInt uilayer, UInt bWidth )							{ m_grid_slice_width_in_mbs_minus1[uilayer] = bWidth; }
		Void setGridSliceHeightInMbsMinus1 ( UInt uilayer, UInt bHeight )						{ m_grid_slice_height_in_mbs_minus1[uilayer] = bHeight; }

		Void setROINum(UInt iDependencyId, UInt iNumROI)  		{ m_aiNumRoi[iDependencyId] = iNumROI; }
		Void setROIID(UInt iDependencyId, UInt* iROIId)
		{
			for (UInt i =0; i < m_aiNumRoi[iDependencyId]; ++i)
			{
				m_aaiRoiID[iDependencyId][i] = iROIId[i];
			}
		}
		Void setSGID(UInt iDependencyId, UInt* iSGId)
		{
			for (UInt i =0; i < m_aiNumRoi[iDependencyId]; ++i)
			{
				m_aaiSGID[iDependencyId][i] = iSGId[i];
			}
		}
		Void setSLID(UInt iDependencyId, UInt* iSGId)
		{
			for (UInt i =0; i < m_aiNumRoi[iDependencyId]; ++i)
			{
				m_aaiSLID[iDependencyId][i] = iSGId[i];
			}
		}

		// JVT-S054 (REPLACE) ->
		//Void setNumSliceMinus1 ( UInt uilayer, UInt bNum ) 										{ m_num_slice_minus1[uilayer] = bNum; }
    Void setNumSliceMinus1 ( UInt uilayer, UInt bNum )
    {
      if ( m_num_slice_minus1[uilayer] != bNum )
      {
        if ( m_first_mb_in_slice[uilayer] != NULL )
        {
          free(m_first_mb_in_slice[uilayer]);
          m_first_mb_in_slice[uilayer] = NULL;
        }
        if ( m_slice_width_in_mbs_minus1[uilayer] != NULL )
        {
          free(m_slice_width_in_mbs_minus1[uilayer]);
          m_slice_width_in_mbs_minus1[uilayer] = NULL;
        }
        if ( m_slice_height_in_mbs_minus1[uilayer] != NULL )
        {
          free(m_slice_height_in_mbs_minus1[uilayer]);
          m_slice_height_in_mbs_minus1[uilayer] = NULL;
        }
      }

      m_num_slice_minus1[uilayer] = bNum;

      if ( m_first_mb_in_slice[uilayer] == NULL )
        m_first_mb_in_slice[uilayer] = (UInt*)malloc((bNum+1)*sizeof(UInt));

      if ( m_slice_width_in_mbs_minus1[uilayer] == NULL )
        m_slice_width_in_mbs_minus1[uilayer] = (UInt*)malloc((bNum+1)*sizeof(UInt));

      if ( m_slice_height_in_mbs_minus1[uilayer] == NULL )
        m_slice_height_in_mbs_minus1[uilayer] = (UInt*)malloc((bNum+1)*sizeof(UInt));

      if ( sizeof(m_slice_id[uilayer]) != (m_frm_width_in_mbs_minus1[uilayer]+1)*(m_frm_height_in_mbs_minus1[uilayer]+1)*sizeof(UInt) )
      {
        free(m_slice_id[uilayer]);
        m_slice_id[uilayer] = NULL;
      }
      if ( m_slice_id[uilayer] == NULL )
        m_slice_id[uilayer] = (UInt*)malloc((m_frm_width_in_mbs_minus1[uilayer]+1)*(m_frm_height_in_mbs_minus1[uilayer]+1)*sizeof(UInt));
    }
		// JVT-S054 (REPLACE) <-

		Void setFirstMbInSlice ( UInt uilayer, UInt uiTar, UInt bNum )							{ m_first_mb_in_slice[uilayer][uiTar] = bNum; }
		Void setSliceWidthInMbsMinus1 ( UInt uilayer, UInt uiTar, UInt bWidth )					{ m_slice_width_in_mbs_minus1[uilayer][uiTar] = bWidth; }
		Void setSliceHeightInMbsMinus1 ( UInt uilayer, UInt uiTar, UInt bHeight )				{ m_slice_height_in_mbs_minus1[uilayer][uiTar] = bHeight; }
		Void setSliceId ( UInt uilayer, UInt uiTar, UInt bId )									{ m_slice_id[uilayer][uiTar] = bId; }
    //JVT-S036 lsj end	
		Void setNumDirectlyDependentLayers ( UInt uilayer, UInt uiNum )						{ m_num_directly_dependent_layers					[uilayer] = uiNum; }
		Void setDirectlyDependentLayerIdDeltaMinus1( UInt uilayer, UInt uiTar, UInt uiDelta ) { m_directly_dependent_layer_id_delta_minus1[uilayer][uiTar] = uiDelta;} ///JVT-S036 lsj
		Void setLayerDependencyInfoSrcLayerIdDelta( UInt uilayer, UInt uiDelta )		  { m_layer_dependency_info_src_layer_id_delta	    [uilayer] = uiDelta;} //JVT-S036 lsj
		Void setNumInitSeqParameterSetMinus1 ( UInt uilayer, UInt uiNum )					{ m_num_init_seq_parameter_set_minus1			[uilayer] = uiNum; }
		Void setInitSeqParameterSetIdDelta ( UInt uilayer, UInt uiSPS, UInt uiTar){ m_init_seq_parameter_set_id_delta				[uilayer][uiSPS] = uiTar;	}
		Void setNumInitPicParameterSetMinus1 ( UInt uilayer, UInt uiNum )					{ m_num_init_pic_parameter_set_minus1			[uilayer] = uiNum; }
		Void setInitPicParameterSetIdDelta ( UInt uilayer, UInt uiPPS, UInt uiTar){ m_init_pic_parameter_set_id_delta				[uilayer][uiPPS] = uiTar; }
		Void setInitParameterSetsInfoSrcLayerIdDelta (UInt uilayer, UInt uiDelta)	{ m_init_parameter_sets_info_src_layer_id_delta[uilayer] = uiDelta; } //JVT-S036 lsj
// BUG_FIX liuhui{
		Void setStdAVCOffset( UInt uiOffset )                                     { m_std_AVC_Offset = uiOffset;}
		UInt getStdAVCOffset()const { return m_std_AVC_Offset; }
// BUG_FIX liuhui}

		UInt getNumLayersMinus1() const {return m_num_layers_minus1;}
		UInt getLayerId ( UInt uilayer ) const { return m_layer_id[uilayer]; }
	 //JVT-S036 lsj start
//		Bool getFGSLayerFlag ( UInt uilayer ) const { return m_fgs_layer_flag[uilayer]; } 
		UInt getSimplePriorityId ( UInt uilayer ) const { return  m_simple_priority_id [uilayer]; }
		Bool getDiscardableFlag	(UInt uilayer) const { return  m_discardable_flag [uilayer]; }
		UInt getTemporalLevel ( UInt uilayer ) const { return m_temporal_level[uilayer]; }
		UInt getDependencyId ( UInt uilayer ) const { return m_dependency_id[uilayer]; }
		UInt getQualityLevel ( UInt uilayer ) const { return m_quality_level[uilayer]; }
	
		Bool getSubPicLayerFlag ( UInt uilayer ) { return m_sub_pic_layer_flag[uilayer]; }
		Bool getSubRegionLayerFlag ( UInt uilayer ) const { return m_sub_region_layer_flag[uilayer]; }
		Bool getIroiSliceDivisionInfoPresentFlag ( UInt uilayer ) const { return m_iroi_slice_division_info_present_flag[uilayer]; } 
		Bool getProfileLevelInfoPresentFlag ( UInt uilayer ) const { return m_profile_level_info_present_flag[uilayer]; }
   //JVT-S036 lsj end
		Bool getBitrateInfoPresentFlag ( UInt uilayer ) const { return m_bitrate_info_present_flag[uilayer]; }
		Bool getFrmRateInfoPresentFlag ( UInt uilayer ) const { return m_frm_rate_info_present_flag[uilayer]; }
		Bool getFrmSizeInfoPresentFlag ( UInt uilayer ) const { return m_frm_size_info_present_flag[uilayer]; }
		Bool getLayerDependencyInfoPresentFlag ( UInt uilayer ) const { return m_layer_dependency_info_present_flag[uilayer]; }
		Bool getInitParameterSetsInfoPresentFlag ( UInt uilayer ) const { return m_init_parameter_sets_info_present_flag[uilayer]; }

		Bool getExactInterlayerPredFlag ( UInt uilayer )	const { return m_exact_interlayer_pred_flag  [uilayer]; }				//JVT-S036 lsj

		UInt getLayerProfileIdc ( UInt uilayer ) const { return m_layer_profile_idc[uilayer]; }
		Bool getLayerConstraintSet0Flag ( UInt uilayer ) const { return m_layer_constraint_set0_flag[uilayer]; }
		Bool getLayerConstraintSet1Flag ( UInt uilayer ) const { return m_layer_constraint_set1_flag[uilayer]; }
		Bool getLayerConstraintSet2Flag ( UInt uilayer ) const { return m_layer_constraint_set2_flag[uilayer]; }
		Bool getLayerConstraintSet3Flag ( UInt uilayer ) const { return m_layer_constraint_set3_flag[uilayer]; }
		UInt getLayerLevelIdc ( UInt uilayer ) const { return m_layer_level_idc[uilayer]; }

	//JVT-S036 lsj start
		UInt getProfileLevelInfoSrcLayerIdDelta ( UInt uilayer) const { return m_profile_level_info_src_layer_id_delta [uilayer];} 
		
		UInt getAvgBitrate ( UInt uilayer ) const { return m_avg_bitrate[uilayer]; }
		UInt getMaxBitrateLayer ( UInt uilayer ) const { return m_max_bitrate_layer[uilayer]; }
		UInt getMaxBitrateDecodedPicture ( UInt uilayer ) const { return m_max_bitrate_decoded_picture[uilayer]; }		
		UInt getMaxBitrateCalcWindow ( UInt uilayer ) const { return m_max_bitrate_calc_window[uilayer]; }
	//JVT-S036 lsj end

		
		UInt getConstantFrmRateIdc ( UInt uilayer ) const { return m_constant_frm_rate_idc[uilayer]; }
		UInt getAvgFrmRate ( UInt uilayer ) const { return m_avg_frm_rate[uilayer]; }
		UInt getFrmRateInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_frm_rate_info_src_layer_id_delta[uilayer]; } //JVT-S036 lsj
		UInt getFrmWidthInMbsMinus1 ( UInt uilayer ) const { return m_frm_width_in_mbs_minus1[uilayer]; }
		UInt getFrmHeightInMbsMinus1 ( UInt uilayer ) const { return m_frm_height_in_mbs_minus1[uilayer]; }
		UInt getFrmSizeInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_frm_size_info_src_layer_id_delta[uilayer]; } //JVT-S036 lsj
		UInt getBaseRegionLayerId ( UInt uilayer ) const { return m_base_region_layer_id[uilayer]; }
		Bool getDynamicRectFlag ( UInt uilayer ) const { return m_dynamic_rect_flag[uilayer]; }
		UInt getHorizontalOffset ( UInt uilayer ) const { return m_horizontal_offset[uilayer]; }
		UInt getVerticalOffset ( UInt uilayer ) const { return m_vertical_offset[uilayer]; }
		UInt getRegionWidth ( UInt uilayer ) const { return m_region_width[uilayer]; }
		UInt getRegionHeight ( UInt uilayer ) const { return m_region_height[uilayer]; }
		UInt getSubRegionInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_sub_region_info_src_layer_id_delta[uilayer]; } ///JVT-S036 lsj
	//JVT-S036 lsj start
		UInt getRoiId ( UInt uilayer ) const { return m_roi_id[uilayer]; } 
		UInt getIroiSliceDivisionType ( UInt uilayer ) const { return m_iroi_slice_division_type[uilayer]; }
		UInt getGridSliceWidthInMbsMinus1 ( UInt uilayer ) const { return m_grid_slice_width_in_mbs_minus1[uilayer]; }
		UInt getGridSliceHeightInMbsMinus1 ( UInt uilayer ) const { return m_grid_slice_height_in_mbs_minus1[uilayer]; }
		UInt getNumSliceMinus1 ( UInt uilayer ) const { return m_num_slice_minus1[uilayer]; }
		UInt getFirstMbInSlice ( UInt uilayer, UInt uiTar )	const { return m_first_mb_in_slice[uilayer][uiTar]; }
		UInt getSliceWidthInMbsMinus1 ( UInt uilayer, UInt uiTar ) const { return m_slice_width_in_mbs_minus1[uilayer][uiTar]; }
		UInt getSliceHeightInMbsMinus1 ( UInt uilayer, UInt uiTar ) const { return m_slice_height_in_mbs_minus1[uilayer][uiTar]; }
		UInt getSliceId ( UInt uilayer, UInt uiTar ) const { return m_slice_id[uilayer][uiTar]; }
	//JVT-S036 lsj end

		UInt getNumDirectlyDependentLayers ( UInt uilayer ) const { return m_num_directly_dependent_layers[uilayer]; }
// BUG_FIX liuhui{
		UInt getNumDirectlyDependentLayerIdDeltaMinus1( UInt uilayer, UInt uiIndex ) const { return m_directly_dependent_layer_id_delta_minus1[uilayer][uiIndex]; } //JVT-S036 lsj
// BUG_FIX liuhui}
		UInt getLayerDependencyInfoSrcLayerIdDelta( UInt uilayer ) const { return m_layer_dependency_info_src_layer_id_delta[uilayer];} //JVT-S036 lsj
		//
		UInt getNumInitSPSMinus1 ( UInt uilayer ) const { return m_num_init_seq_parameter_set_minus1[uilayer]; }
		UInt getNumInitPPSMinus1 ( UInt uilayer ) const { return m_num_init_pic_parameter_set_minus1[uilayer]; }
// BUG_FIX liuhui{
		UInt getInitSPSIdDelta ( UInt uilayer, UInt uiIndex ) const { return m_init_seq_parameter_set_id_delta[uilayer][uiIndex]; }
		UInt getInitPPSIdDelta ( UInt uilayer, UInt uiIndex ) const { return m_init_pic_parameter_set_id_delta[uilayer][uiIndex]; }
// BUG_FIX liuhui}
		UInt getInitParameterSetsInfoSrcLayerIdDelta ( UInt uilayer ) const { return m_init_parameter_sets_info_src_layer_id_delta[uilayer]; } //JVT-S036 lsj

	private:
// BUG_FIX liuhui{
		UInt m_std_AVC_Offset;
// BUG_FIX liuhui}
		UInt m_num_layers_minus1;
		UInt m_layer_id[MAX_SCALABLE_LAYERS];
	//JVT-S036 lsj start
		//Bool m_fgs_layer_flag[MAX_SCALABLE_LAYERS];  
		UInt m_simple_priority_id[MAX_SCALABLE_LAYERS];  
		Bool m_discardable_flag[MAX_SCALABLE_LAYERS];
		UInt m_temporal_level[MAX_SCALABLE_LAYERS];
		UInt m_dependency_id[MAX_SCALABLE_LAYERS];
		UInt m_quality_level[MAX_SCALABLE_LAYERS];

		Bool m_sub_pic_layer_flag[MAX_SCALABLE_LAYERS];
		Bool m_sub_region_layer_flag[MAX_SCALABLE_LAYERS];
		Bool m_iroi_slice_division_info_present_flag[MAX_SCALABLE_LAYERS]; 
		Bool m_profile_level_info_present_flag[MAX_SCALABLE_LAYERS];
	//JVT-S036 lsj end
		Bool m_bitrate_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_frm_rate_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_frm_size_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_dependency_info_present_flag[MAX_SCALABLE_LAYERS];
		Bool m_init_parameter_sets_info_present_flag[MAX_SCALABLE_LAYERS];

		Bool m_exact_interlayer_pred_flag[MAX_SCALABLE_LAYERS];  //JVT-S036 lsj

		UInt m_layer_profile_idc[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set0_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set1_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set2_flag[MAX_SCALABLE_LAYERS];
		Bool m_layer_constraint_set3_flag[MAX_SCALABLE_LAYERS];
		UInt m_layer_level_idc[MAX_SCALABLE_LAYERS];
        
	//JVT-S036 lsj start
		UInt m_profile_level_info_src_layer_id_delta[MAX_SCALABLE_LAYERS]; //
	


		UInt m_avg_bitrate[MAX_SCALABLE_LAYERS];
		UInt m_max_bitrate_layer[MAX_SCALABLE_LAYERS];//
		UInt m_max_bitrate_decoded_picture[MAX_SCALABLE_LAYERS];//
		UInt m_max_bitrate_calc_window[MAX_SCALABLE_LAYERS];//

		UInt m_constant_frm_rate_idc[MAX_SCALABLE_LAYERS];
		UInt m_avg_frm_rate[MAX_SCALABLE_LAYERS];

		UInt m_frm_rate_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//
	
		UInt m_frm_width_in_mbs_minus1[MAX_SCALABLE_LAYERS];
		UInt m_frm_height_in_mbs_minus1[MAX_SCALABLE_LAYERS];

		UInt m_frm_size_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//

		UInt m_base_region_layer_id[MAX_SCALABLE_LAYERS];
		Bool m_dynamic_rect_flag[MAX_SCALABLE_LAYERS];
		UInt m_horizontal_offset[MAX_SCALABLE_LAYERS];
		UInt m_vertical_offset[MAX_SCALABLE_LAYERS];
		UInt m_region_width[MAX_SCALABLE_LAYERS];
		UInt m_region_height[MAX_SCALABLE_LAYERS];

		UInt m_sub_region_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//

		UInt m_roi_id[MAX_SCALABLE_LAYERS]; //

		UInt m_iroi_slice_division_type[MAX_SCALABLE_LAYERS]; //
		UInt m_grid_slice_width_in_mbs_minus1[MAX_SCALABLE_LAYERS]; //
		UInt m_grid_slice_height_in_mbs_minus1[MAX_SCALABLE_LAYERS]; //
		UInt m_num_slice_minus1[MAX_SCALABLE_LAYERS];//
		// JVT-S054 (REPLACE) ->
    /*
		UInt m_first_mb_in_slice[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
		UInt m_slice_width_in_mbs_minus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
		UInt m_slice_height_in_mbs_minus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
		UInt m_slice_id[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//
    */
		UInt* m_first_mb_in_slice[MAX_SCALABLE_LAYERS];//
		UInt* m_slice_width_in_mbs_minus1[MAX_SCALABLE_LAYERS];//
		UInt* m_slice_height_in_mbs_minus1[MAX_SCALABLE_LAYERS];//
		UInt* m_slice_id[MAX_SCALABLE_LAYERS];//
		// JVT-S054 (REPLACE) <-
// BUG_FIX liuhui{
		UInt m_num_directly_dependent_layers[MAX_SCALABLE_LAYERS];
		UInt m_directly_dependent_layer_id_delta_minus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];//

		UInt m_layer_dependency_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//

		UInt m_num_init_seq_parameter_set_minus1[MAX_SCALABLE_LAYERS];
		UInt m_init_seq_parameter_set_id_delta[MAX_SCALABLE_LAYERS][32];
		UInt m_num_init_pic_parameter_set_minus1[MAX_SCALABLE_LAYERS];
		UInt m_init_pic_parameter_set_id_delta[MAX_SCALABLE_LAYERS][256];
// BUG_FIX liuhui}
		UInt m_init_parameter_sets_info_src_layer_id_delta[MAX_SCALABLE_LAYERS];//
	//JVT-S036 lsj end

		UInt m_aiNumRoi[MAX_SCALABLE_LAYERS];
		UInt m_aaiRoiID[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
		UInt m_aaiSGID[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
		UInt m_aaiSLID[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
	};

	class H264AVCCOMMONLIB_API SubPicSei : public SEIMessage
	{
	protected:
		SubPicSei ();
		~SubPicSei();

	public:
		static ErrVal create	( SubPicSei*&				rpcSeiMessage );
		ErrVal				write		( HeaderSymbolWriteIf*	pcWriteIf );
		ErrVal				read		( HeaderSymbolReadIf*		pcReadIf  );	

		UInt getLayerId	()					const	{ return m_uiLayerId;				}
		Void setLayerId ( UInt uiLayerId) { m_uiLayerId = uiLayerId;	}

	private:
		UInt m_uiLayerId;
	};

  class H264AVCCOMMONLIB_API MotionSEI : public SEIMessage
  {

  protected:
    MotionSEI();
    ~MotionSEI();

  public:

    UInt m_num_slice_groups_in_set_minus1;
    UInt m_slice_group_id[8];
    Bool m_exact_sample_value_match_flag;
    Bool m_pan_scan_rect_flag;

    static ErrVal create  ( MotionSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        setSliceGroupId(UInt id);
	UInt          getSliceGroupId(){return m_slice_group_id[0];}
  };
  
  //{{Quality level estimation and modified truncation- JVTO044 and m12007
  //France Telecom R&D-(nathalie.cammas@francetelecom.com)
  class H264AVCCOMMONLIB_API QualityLevelSEI : public SEIMessage
  {
	protected:
    QualityLevelSEI ();
    ~QualityLevelSEI();

  public:
    static ErrVal create  ( QualityLevelSEI*&         rpcSeiMessage );
    ErrVal        write   ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read    ( HeaderSymbolReadIf*   pcReadIf );
	
	UInt		 getNumLevel() { return m_uiNumLevels;}
	Void		 setNumLevel(UInt ui) { m_uiNumLevels = ui;}
	UInt		 getDeltaBytesRateOfLevel(UInt ui) { return m_auiDeltaBytesRateOfLevel[ui];}
	Void		 setDeltaBytesRateOfLevel(UInt uiIndex, UInt ui) { m_auiDeltaBytesRateOfLevel[uiIndex] = ui;}
	UInt		 getQualityLevel(UInt ui) { return m_auiQualityLevel[ui];}
	Void		 setQualityLevel(UInt uiIndex, UInt ui) { m_auiQualityLevel[uiIndex] = ui;}
	UInt		 getDependencyId() { return m_uiDependencyId;}
	Void		 setDependencyId( UInt ui) { m_uiDependencyId = ui;}

  private:
	  UInt m_auiQualityLevel[MAX_NUM_RD_LEVELS];
	  UInt m_auiDeltaBytesRateOfLevel[MAX_NUM_RD_LEVELS];
	  UInt m_uiNumLevels;
	  UInt m_uiDependencyId;
  };
  //}}Quality level estimation and modified truncation- JVTO044 and m12007


  class H264AVCCOMMONLIB_API NonRequiredSei : public SEIMessage
  {
  protected:
	  NonRequiredSei ();
	  ~NonRequiredSei();

  public:
	  static ErrVal create	(NonRequiredSei*&			rpcSeiMessage);
	  ErrVal		destroy ();  
	  ErrVal		write	(HeaderSymbolWriteIf*		pcWriteIf);
	  ErrVal		read	(HeaderSymbolReadIf*		pcReadIf);

	  UInt			getNumInfoEntriesMinus1()					const{ return m_uiNumInfoEntriesMinus1;}
	  UInt			getEntryDependencyId(UInt uiLayer)			const{ return m_uiEntryDependencyId[uiLayer];}
	  UInt			getNumNonRequiredPicsMinus1(UInt uiLayer)	const{ return m_uiNumNonRequiredPicsMinus1[uiLayer];}
	  UInt			getNonRequiredPicDependencyId(UInt uiLayer, UInt uiNonRequiredLayer)	const{ return m_uiNonRequiredPicDependencyId[uiLayer][uiNonRequiredLayer];}
	  UInt			getNonRequiredPicQulityLevel(UInt uiLayer, UInt uiNonRequiredLayer)		const{ return m_uiNonRequiredPicQulityLevel[uiLayer][uiNonRequiredLayer];}
	  UInt			getNonRequiredPicFragmentOrder(UInt uiLayer, UInt uiNonRequiredLayer)	const{ return m_uiNonRequiredPicFragmentOrder[uiLayer][uiNonRequiredLayer];}


	  Void			setNumInfoEntriesMinus1(UInt ui)					{ m_uiNumInfoEntriesMinus1 = ui;}
	  Void			setEntryDependencyId(UInt uiLayer, UInt ui)			{ m_uiEntryDependencyId[uiLayer] = ui;}
	  Void			setNumNonRequiredPicsMinus1(UInt uiLayer, UInt ui)	{ m_uiNumNonRequiredPicsMinus1[uiLayer] = ui;}
	  Void			setNonNonRequiredPicDependencyId(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)		{m_uiNonRequiredPicDependencyId[uiLayer][uiNonRequiredLayer] = ui;}
	  Void			setNonNonRequiredPicQulityLevel(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)			{m_uiNonRequiredPicQulityLevel[uiLayer][uiNonRequiredLayer] = ui;}
	  Void			setNonNonRequiredPicFragmentOrder(UInt uiLayer, UInt uiNonRequiredLayer, UInt ui)		{m_uiNonRequiredPicFragmentOrder[uiLayer][uiNonRequiredLayer] = ui;}


  private:
	  UInt		m_uiNumInfoEntriesMinus1;
	  UInt		m_uiEntryDependencyId[MAX_NUM_INFO_ENTRIES];
	  UInt		m_uiNumNonRequiredPicsMinus1[MAX_NUM_INFO_ENTRIES];
	  UInt		m_uiNonRequiredPicDependencyId[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
	  UInt		m_uiNonRequiredPicQulityLevel[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
	  UInt		m_uiNonRequiredPicFragmentOrder[MAX_NUM_INFO_ENTRIES][MAX_NUM_NON_REQUIRED_PICS];
  };//shenqiu 05-09-15

  // JVT-S080 LMI {
  class H264AVCCOMMONLIB_API ScalableSeiLayersNotPresent: public SEIMessage
  {
  protected:
      ScalableSeiLayersNotPresent ();
	 ~ScalableSeiLayersNotPresent();

  public:
      static ErrVal create ( ScalableSeiLayersNotPresent*&			rpcSeiMessage);
      ErrVal write				 ( HeaderSymbolWriteIf	*pcWriteIf);
      ErrVal read					 ( HeaderSymbolReadIf		*pcReadIf);
      Void setNumLayers( UInt ui )																				{ m_uiNumLayers = ui;	}
      Void setLayerId ( UInt uiLayer, UInt uiId )																{ m_auiLayerId															[uiLayer] = uiId; }
	  Void setOutputFlag ( Bool bFlag )  { m_bOutputFlag = bFlag; }

      UInt getNumLayers() const {return m_uiNumLayers;}
      UInt getLayerId ( UInt uiLayer ) const { return m_auiLayerId[uiLayer]; }
	  Bool getOutputFlag ( ) const { return m_bOutputFlag; }
      static UInt m_uiLeftNumLayers;
      static UInt m_auiLeftLayerId[MAX_SCALABLE_LAYERS];

  private:
      UInt m_uiNumLayers;
      UInt m_auiLayerId[MAX_SCALABLE_LAYERS];
	  Bool m_bOutputFlag;

	};

  class H264AVCCOMMONLIB_API ScalableSeiDependencyChange: public SEIMessage
  {
  protected:
      ScalableSeiDependencyChange ();
	 ~ScalableSeiDependencyChange();

  public:
      static ErrVal create ( ScalableSeiDependencyChange*&			rpcSeiMessage);
      ErrVal write				 ( HeaderSymbolWriteIf	*pcWriteIf);
      ErrVal read					 ( HeaderSymbolReadIf		*pcReadIf);
      Void setNumLayersMinus1( UInt ui )																				{ m_uiNumLayersMinus1 = ui;	}
      Void setLayerId ( UInt uiLayer, UInt uiId )																{ m_auiLayerId															[uiLayer] = uiId; }
	  Void setLayerDependencyInfoPresentFlag ( UInt uiLayer, Bool bFlag ) { m_abLayerDependencyInfoPresentFlag[uiLayer] = bFlag; }
      Void setNumDirectDependentLayers ( UInt uiLayer, UInt ui ) { m_auiNumDirectDependentLayers[uiLayer] = ui; }
	  Void setDirectDependentLayerIdDeltaMinus1( UInt uiLayer, UInt uiDirectLayer, UInt uiIdDeltaMinus1 )  { m_auiDirectDependentLayerIdDeltaMinus1[uiLayer][uiDirectLayer] = uiIdDeltaMinus1; }
	  Void setLayerDependencyInfoSrcLayerIdDeltaMinus1 ( UInt uiLayer, UInt uiIdDeltaMinus1 ) { m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[uiLayer] = uiIdDeltaMinus1; }
	  Void setOutputFlag ( Bool bFlag )  { m_bOutputFlag = bFlag; }

      UInt getNumLayersMinus1() const {return m_uiNumLayersMinus1;}
      UInt getLayerId ( UInt uiLayer ) const { return m_auiLayerId[uiLayer]; }
      UInt getNumDirectDependentLayers ( UInt uiLayer ) const { return m_auiNumDirectDependentLayers[uiLayer]; }
	  UInt getDirectDependentLayerIdDeltaMinus1( UInt uiLayer, UInt uiDirectLayer ) const { return m_auiDirectDependentLayerIdDeltaMinus1[uiLayer][uiDirectLayer]; }
	  UInt getLayerDependencyInfoSrcLayerIdDeltaMinus1 ( UInt uiLayer ) const { return m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[uiLayer]; }
	  Bool getLayerDependencyInfoPresentFlag ( UInt uiLayer ) const { return m_abLayerDependencyInfoPresentFlag[uiLayer]; }
	  Bool getOutputFlag ( ) const { return m_bOutputFlag; }

  private:
      UInt m_uiNumLayersMinus1;
      UInt m_auiLayerId[MAX_SCALABLE_LAYERS];
      UInt m_auiNumDirectDependentLayers[MAX_SCALABLE_LAYERS];
      UInt m_auiDirectDependentLayerIdDeltaMinus1[MAX_SCALABLE_LAYERS][MAX_SCALABLE_LAYERS];
      UInt m_auiLayerDependencyInfoSrcLayerIdDeltaMinus1[MAX_SCALABLE_LAYERS];
	  Bool m_abLayerDependencyInfoPresentFlag[MAX_SCALABLE_LAYERS];
	  Bool m_bOutputFlag;
	};

  // JVT-S080 LMI }
// JVT-T073 {
#define MAX_PICTURES_IN_ACCESS_UNIT 50
  class H264AVCCOMMONLIB_API ScalableNestingSei : public SEIMessage
  {
  protected:
    ScalableNestingSei()
      : SEIMessage(SCALABLE_NESTING_SEI)
      , m_bAllPicturesInAuFlag  (0)
	  , m_uiNumPictures         (0)
	  , m_pcSEIMessage          (NULL)
    {}

  public:
    static ErrVal create( ScalableNestingSei*&  rpcSEIMessage );
	ErrVal		  destroy();  
    ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf );
    ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );
    ErrVal        init  ( Bool                  m_bAllPicturesInAuFlag,
	                      UInt                  m_uiNumPictures,
	                      UInt*                 m_auiDependencyId,
	                      UInt*                 m_auiQualityLevel 
						);

    Bool getAllPicturesInAuFlag()  const { return m_bAllPicturesInAuFlag; }
    UInt getNumPictures()          const { return m_uiNumPictures; }
	UInt getDependencyId( UInt uiIndex ) { return m_auiDependencyId[uiIndex]; }
	UInt getQualityLevel( UInt uiIndex ) { return m_auiQualityLevel[uiIndex]; }

	Void setAllPicturesInAuFlag( Bool bFlag ) { m_bAllPicturesInAuFlag = bFlag; }
	Void setNumPictures( UInt uiNum ) { m_uiNumPictures = uiNum; }
	Void setDependencyId( UInt uiIndex, UInt uiValue ) { m_auiDependencyId[uiIndex] = uiValue; }
	Void setQualityLevel( UInt uiIndex, UInt uiValue ) { m_auiQualityLevel[uiIndex] = uiValue; }

  private:
	  Bool  m_bAllPicturesInAuFlag;
	  UInt  m_uiNumPictures;
	  UInt  m_auiDependencyId[MAX_PICTURES_IN_ACCESS_UNIT];
	  UInt  m_auiQualityLevel[MAX_PICTURES_IN_ACCESS_UNIT];
	  SEIMessage *m_pcSEIMessage;
  };
  //scene_info is taken as en example
  class H264AVCCOMMONLIB_API SceneInfoSei : public SEIMessage
  {
  protected:
	  SceneInfoSei() : SEIMessage(SCENE_INFO_SEI)
	  {}
  public:
	  static ErrVal create( SceneInfoSei*& rpcSceneInfoSei );
	  ErrVal		destroy ();  
	  ErrVal        write ( HeaderSymbolWriteIf*  pcWriteIf);
      ErrVal        read  ( HeaderSymbolReadIf*   pcReadIf );

	  Bool getSceneInfoPresentFlag() const { return m_bSceneInfoPresentFlag; }
	  UInt getSceneId()              const { return m_uiSceneId; }
	  UInt getSceneTransitionType()  const { return m_uiSceneTransitionType; }
	  UInt getSecondSceneId()        const { return m_uiSecondSceneId; }
	  Void setSceneInfoPresentFlag( Bool bFlag )          { m_bSceneInfoPresentFlag = bFlag; }
	  Void setSceneId( UInt uiSceneId )                   { m_uiSceneId = uiSceneId; }
	  Void setSceneTransitionType( UInt uiTransitionType) { m_uiSceneTransitionType = uiTransitionType; }
	  Void setSecondSceneId( UInt uiSecondId )            { m_uiSecondSceneId = uiSecondId; }
  private:
	  Bool m_bSceneInfoPresentFlag;
	  UInt m_uiSceneId;
	  UInt m_uiSceneTransitionType;
	  UInt m_uiSecondSceneId;
  };
  // JVT-T073 }

  typedef MyList<SEIMessage*> MessageList;
  
  static ErrVal read  ( HeaderSymbolReadIf*   pcReadIf,
                        MessageList&          rcSEIMessageList );
  static ErrVal write ( HeaderSymbolWriteIf*  pcWriteIf,
                        HeaderSymbolWriteIf*  pcWriteTestIf,
                        MessageList*          rpcSEIMessageList );
  //JVT-T073 {
  static ErrVal writeNesting        ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      MessageList*          rpcSEIMessageList );
  static ErrVal xWriteNesting       ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      SEIMessage*           pcSEIMessage,
									  UInt&                 uiBits );
 //JVT-T073 }

protected:
  static ErrVal xRead               ( HeaderSymbolReadIf*   pcReadIf,
                                      SEIMessage*&          rpcSEIMessage ); 
  static ErrVal xWrite              ( HeaderSymbolWriteIf*  pcWriteIf,
                                      HeaderSymbolWriteIf*  pcWriteTestIf,
                                      SEIMessage*           pcSEIMessage );
  static ErrVal xWritePayloadHeader ( HeaderSymbolWriteIf*  pcWriteIf,
                                      MessageType           eMessageType,
                                      UInt                  uiSize );
  static ErrVal xReadPayloadHeader  ( HeaderSymbolReadIf*   pcReadIf,
                                      MessageType&          reMessageType,
                                      UInt&                 ruiSize);
  static ErrVal xCreate             ( SEIMessage*&          rpcSEIMessage,
                                      MessageType           eMessageType,
                                      UInt                  uiSize ); 
public:


};

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif

H264AVC_NAMESPACE_END


#endif // !defined(AFX_SEI_H__06FFFAD0_FB36_4BF0_9392_395C7389C1F4__INCLUDED_)
