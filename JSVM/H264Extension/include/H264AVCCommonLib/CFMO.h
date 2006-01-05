
/*!
 ***************************************************************************
 *
 * \file CFMO.h
 *
 * \brief
 *    Support for Flexilble Macroblock Ordering (FMO)
 *
 * \date
 *    10 May, 2005
 *
 * \author
 *    Tae Meon Bae	heartles@icu.ac.kr
 *
 **************************************************************************/


#if !defined(AFX_CFMO_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
#define AFX_CFMO_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_



H264AVC_NAMESPACE_BEGIN

const unsigned MAXnum_slice_groups_minus1 =8;
const int null= 0;


class ImageParameter
{
public:
	unsigned PicSizeInMbs;
	unsigned int field_pic_flag;
	unsigned PicWidthInMbs;  
	unsigned PicHeightInMapUnits;
	int slice_group_change_cycle;

	void SetImageParameters(unsigned PicSizeInMbs,	unsigned int field_pic_flag, 	unsigned PicWidthInMbs,unsigned PicHeightInMapUnits,	int slice_group_change_cycle)
	{

	}
};


class FMO_PPS
{

public:
	
	unsigned num_slice_group_map_units_minus1;
	unsigned num_slice_groups_minus1;
	unsigned slice_group_map_type;
	unsigned run_length_minus1[MAXnum_slice_groups_minus1];
	unsigned top_left[MAXnum_slice_groups_minus1];		
	unsigned bottom_right[MAXnum_slice_groups_minus1];
	bool slice_group_change_direction_flag;
	unsigned slice_group_change_rate_minus1;
	unsigned int slice_group_id[MAXnum_slice_groups_minus1];

public:
	FMO_PPS():num_slice_group_map_units_minus1(0)
		,num_slice_groups_minus1(0)
		,slice_group_map_type(0)
		,slice_group_change_direction_flag(0)
		,slice_group_change_rate_minus1(0){};


	void copy_run_length_minus1(unsigned* Run_length_minus1)
	{
		assert(MAXnum_slice_groups_minus1>0);
		for(int i=0;i<MAXnum_slice_groups_minus1;i++)
		{
			run_length_minus1[i] = Run_length_minus1[i];
		}
	};

	void copy_top_left(unsigned* Top_left)
	{
		assert(MAXnum_slice_groups_minus1>0);
		for(int i=0;i<num_slice_groups_minus1;i++)
		{
			top_left[i] = Top_left[i];
		}
	}

	void copy_bottom_right(unsigned* Bottom_right)
	{
		assert(MAXnum_slice_groups_minus1>0);
		for(int i=0;i<num_slice_groups_minus1;i++)
		{
			bottom_right[i] = Bottom_right[i];
		}
	}

	void copy_slice_group_id(unsigned int* Slice_group_id)
	{
		assert(MAXnum_slice_groups_minus1>0);
		for(int i=0;i<num_slice_groups_minus1;i++)
			slice_group_id[i] = Slice_group_id[i];
	}

};


class FMO_SPS
{
public:
	FMO_SPS():pic_height_in_map_units_minus1(0),pic_width_in_mbs_minus1(0),frame_mbs_only_flag(1),mb_adaptive_frame_field_flag(0){};
	unsigned pic_height_in_map_units_minus1;
	unsigned pic_width_in_mbs_minus1;
	bool frame_mbs_only_flag;
	bool mb_adaptive_frame_field_flag;
};

class FMO
{

	int NumberOfSliceGroups_;    // the number of slice groups  (0 == scan order, 7 == maximum)

	int *MbToSliceGroupMap_;
	int *MapUnitToSliceGroupMap_; 
	unsigned PicSizeInMapUnits_ ;
	int *numMbInSliceGroup_;
	
	//enc
	int FirstMBInSlice[MAXnum_slice_groups_minus1];

public :
	ImageParameter img_;         //!< image parameters
	FMO_PPS pps_;
	FMO_SPS sps_;


public:
	
	FMO():MbToSliceGroupMap_(NULL),MapUnitToSliceGroupMap_(NULL),numMbInSliceGroup_(NULL)	{	};	
	~FMO(){	finit();};		
	int initImageParameter (ImageParameter *img){return 1;};	
	int init (FMO_PPS* Pps, FMO_SPS* Sps);
	int finit (); //destroy
	int getNumberOfSliceGroup();
	int getLastMBOfPicture();
	int getLastMBInSliceGroup(int SliceGroup);
	int getSliceGroupId (int mb);
	int getNextMBNr (int CurrentMbNr);
	int getNumMbInSliceGroup(int sliceGroupID);

	//--Enc
	int StartPicture ();

	int EndPicture ();
	
	int getPreviousMBNr (int CurrentMbNr);
	int getFirstMBOfSliceGroup (int SliceGroupID);
	int getLastCodedMBOfSliceGroup (int SliceGroupID);
	int getFirstMacroblockInSlice ( int SliceGroup);
	int SliceGroupCompletelyCoded( int SliceGroupID);

	void setLastMacroblockInSlice ( int mb);


  //--ICU/ETRI FMO Implementation
	unsigned int GetYTopLeft(int iGroup);
	unsigned int GetXTopLeft(int iGroup);
	unsigned int GetYBottomRight(int iGroup);
	unsigned int GetXBottomRight(int iGroup);


private:
	void GenerateType0MapUnitMap ();
	void GenerateType1MapUnitMap ();
	void GenerateType2MapUnitMap ();
	void GenerateType3MapUnitMap ();
	void GenerateType4MapUnitMap ();
	void GenerateType5MapUnitMap ();
	void GenerateType6MapUnitMap ();
	
	int GenerateMapUnitToSliceGroupMap();
	int GenerateMbToSliceGroupMap();
	
	void printFmoMaps();
	void mallocMbToSliceGroupMap();
	int initMapUnitToSliceGroupMap();

	unsigned GetNumSliceGroupMapUnits();


	void InitFirstMBsInSlices();

	void calcMbNumInSliceGroup();
};


H264AVC_NAMESPACE_END


#endif //!defined(AFX_CFMO_H__CBFE313E_2382_4ECC_9D41_416668E3507D__INCLUDED_)
