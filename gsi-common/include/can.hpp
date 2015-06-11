#ifndef CAN_HPP_INCLUDED
#define CAN_HPP_INCLUDED


#include "candefs.h"
#include <gerror.h>

//forward references
class GSIRecord;


#pragma pack(1)
typedef struct can_frame_s
{
  wxUint32 id;
  GSIChar buffer[CANMaxData]; //max buffer length of a CAN frame is 8 bytes
  GSIChar length;
  GSIChar flags;
}CANFRAME, *PCANFRAME ;
#pragma pack()

class  CANFrame
{
public:
    explicit CANFrame();
    CANFrame(wxUint32 id,GSIChar *buff,GSIChar length,GSIChar flags);
    virtual ~CANFrame();

    wxUint32 GetID()const { return(m_frame.id);}
    GSIChar GetLength()const{return(m_frame.length);}
    GSIChar GetFlags() const{return(m_frame.flags);}
    const GSIChar GetBufferValue(int index) const;
	PCANFRAME  const GetFrame() {return(&m_frame);}

    const wxString  GetBufferStr() const {return(m_bufferStr);}
	errorType SetBuffer(int index,GSIChar value);
    errorType SetLength(GSIChar l);
	errorType SetID(wxUint32 n );
    errorType SetFlags(GSIChar new_flags);

protected:
  CANFRAME m_frame;
private:
   const wxString m_bufferStr;
};

class GSICANFrame :public CANFrame
{
public:
    GSICANFrame();
    GSICANFrame(wxUint32 id,GSIChar *buff,GSIChar length,GSIChar flags);
    explicit GSICANFrame(GSIRecord &r);
	virtual ~GSICANFrame();

    GSIChar GetOriginNode() const {return(m_originNode);}		//node the message originated from
    void SetOriginNode (unsigned char  n){m_originNode=n;}
    void MakeNode() {m_originNode=GetID()/GSICANFrameBase;}
    int	GetGSIID() const{return(m_gsiID);}			//the id divided by (node*GSICANBase)
	void SetGSIID(int n) {m_gsiID=n;}
    //GSIChar GetData(size_t index)		            //See GetBufferValue

//these don't need to be const, the funcs will return new wxStrings
    const wxString GetIDString()            const {return(m_IDString); }
    const wxString GetOriginNodeString()    const {return(m_OriginNodeString); }
    const wxString GetLengthString()        const {return(m_LengthString);}
    const wxString GetGSIIDString()         const {return(m_GSIIDString);}
    const wxString GetFlagsString()         const {return(m_FlagsString);}
    const wxString GetDataString()          const {return(m_DataString);}
    const wxString GetSubfuncString()       const {return(m_SubfuncString);}
    const wxString GetInformationString1()  const {return m_InfoString1;}
    const wxString GetInformationString2()  const {return m_InfoString2;}

    void   SetStrings();                //Set the strings using the underlying CANFrame data
    void   SetIDString(const wxString &s) {m_IDString=s;}
    void   SetOriginNodeString(const wxString &s) {m_OriginNodeString=s;}    //Does this copy the string or the reference to the string??
    void   SetLengthString(const wxString &s) {m_LengthString=s;}
    void   SetGSIIDString(const wxString &s) {m_GSIIDString=s;}
    void   SetFlagsString(const wxString &s) {m_FlagsString=s;}
    void   SetDataString(const wxString &s) {m_DataString=s;}
    void   SetSubfuncString(const wxString &s) {m_SubfuncString=s;}           //filled in by GSIStringRecord
    void   SetInformationString1(const wxString &s) {m_InfoString1=s;}
    void   SetInformationString2(const wxString &s) {m_InfoString2=s;}

    GSIChar FindOriginNode();       //A temporary function to try and establish the node of the sender
protected:

                                    //Not all CAN frames encode this at present. This function does what it can
private:
    GSIChar m_originNode;
	int m_gsiID;
    bool m_filled;
    wxString m_IDString;         //id of CAN frame
    wxString m_OriginNodeString;
    wxString m_LengthString;
    wxString m_GSIIDString;      //id of the GSI function (m_Id/(node*4096))
    wxString m_FlagsString;
    wxString m_DataString;
    wxString m_SubfuncString;
    wxString m_InfoString1;
    wxString m_InfoString2;

};



#endif
