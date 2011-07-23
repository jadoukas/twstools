#include "tws_meta.h"
#include "tws_xml.h"

#include "twsUtil.h"
#include "debug.h"

#include <QtCore/QStringList>
#include <QtCore/QHash>


#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <limits.h>








/// stupid static helper
std::string ibDate2ISO( const std::string &ibDate )
{
	struct tm tm;
	char buf[255];
	char *tmp;
	
	memset(&tm, 0, sizeof(struct tm));
	tmp = strptime( ibDate.c_str(), "%Y%m%d", &tm);
	if( tmp != NULL && *tmp == '\0' ) {
		strftime(buf, sizeof(buf), "%F", &tm);
		return buf;
	}
	
	memset(&tm, 0, sizeof(struct tm));
	tmp = strptime( ibDate.c_str(), "%Y%m%d%t%H:%M:%S", &tm);
	if(  tmp != NULL && *tmp == '\0' ) {
		strftime(buf, sizeof(buf), "%F %T", &tm);
		return buf;
	}
	
	return "";
}

typedef const char* string_pair[2];

const string_pair short_wts_[]= {
	{"TRADES", "T"},
	{"MIDPOINT", "M"},
	{"BID", "B"},
	{"ASK", "A"},
	{"BID_ASK", "BA"},
	{"HISTORICAL_VOLATILITY", "HV"},
	{"OPTION_IMPLIED_VOLATILITY", "OIV"},
	{"OPTION_VOLUME", "OV"},
	{NULL, "NNN"}
};

const string_pair short_bar_size_[]= {
	{"1 secs",   "s01"},
	{"5 secs",   "s05"},
	{"15 secs",  "s15"},
	{"30 secs",  "s30"},
	{"1 min",    "m01"},
	{"2 mins",   "m02"},
	{"3 mins",   "m03"},
	{"5 mins",   "m05"},
	{"15 mins",  "m15"},
	{"30 mins",  "m30"},
	{"1 hour",   "h01"},
	{"4 hour",   "h04"},
	{"1 day",    "eod"},
	{"1 week",   "w01"},
	{"1 month",  "x01"},
	{"3 months", "x03"},
	{"1 year",   "y01"},
	{NULL, "00N"}
};

const char* short_string( const string_pair* pairs, const char* s_short )
{
	const string_pair *i = pairs;
	while( (*i)[0] != 0 ) {
		if( strcmp((*i)[0], s_short)==0 ) {
			return (*i)[1];
		}
		i++;
	}
	return (*i)[1];
}

const char* short_wts( const char* wts )
{
	return short_string( short_wts_, wts );
}

const char* short_bar_size( const char* bar_size )
{
	return short_string( short_bar_size_, bar_size );
}





ContractDetailsRequest * ContractDetailsRequest::fromXml( xmlNodePtr xn )
{
	ContractDetailsRequest *cdr = new ContractDetailsRequest();
	
	for( xmlNodePtr p = xn->children; p!= NULL; p=p->next) {
		if( p->type == XML_ELEMENT_NODE
			&& strcmp((char*)p->name, "reqContract") == 0 )  {
			IB::Contract c;
			conv_xml2ib( &c, p);
			cdr->initialize(c);
		}
	}
	
	return cdr;
}

const IB::Contract& ContractDetailsRequest::ibContract() const
{
	return _ibContract;
}

bool ContractDetailsRequest::initialize( const IB::Contract& c )
{
	_ibContract = c;
	return true;
}








bool HistRequest::initialize( const IB::Contract& c, const std::string &e,
	const std::string &d, const std::string &b,
	const std::string &w, int u, int f )
{
	_ibContract = c;
	_endDateTime = e;
	_durationStr = d;
	_barSizeSetting = b;
	_whatToShow = w;
	_useRTH = u;
	_formatDate = f;
	return true;
}


std::string HistRequest::toString() const
{
	char buf_c[512];
	char buf_a[1024];
	snprintf( buf_c, sizeof(buf_c), "%s\t%s\t%s\t%s\t%s\t%g\t%s",
		_ibContract.symbol.c_str(),
		_ibContract.secType.c_str(),
		_ibContract.exchange.c_str(),
		_ibContract.currency.c_str(),
		_ibContract.expiry.c_str(),
		_ibContract.strike,
		_ibContract.right.c_str() );
	
	snprintf( buf_a, sizeof(buf_a), "%s\t%s\t%s\t%s\t%d\t%d\t%s",
		_endDateTime.c_str(),
		_durationStr.c_str(),
		_barSizeSetting.c_str(),
		_whatToShow.c_str(),
		_useRTH,
		_formatDate,
		buf_c );
	
	return std::string(buf_a);
}


void HistRequest::clear()
{
	_ibContract = IB::Contract();
	_whatToShow.clear();
}



#define GET_ATTR_STRING( _struct_, _name_, _attr_ ) \
	tmp = (char*) xmlGetProp( node, (const xmlChar*) _name_ ); \
	_struct_->_attr_ = tmp ? std::string(tmp) \
		: dflt._attr_; \
	free(tmp)

#define GET_ATTR_INT( _struct_, _name_, _attr_ ) \
	tmp = (char*) xmlGetProp( node, (const xmlChar*) _name_ ); \
	_struct_->_attr_ = tmp ? atoi( tmp ) : dflt._attr_; \
	free(tmp)

#define GET_ATTR_DOUBLE( _struct_, _name_, _attr_ ) \
	tmp = (char*) xmlGetProp( node, (const xmlChar*) _name_ ); \
	_struct_->_attr_ = tmp ? atof( tmp ) : dflt._attr_; \
	free(tmp)
#define GET_ATTR_BOOL( _struct_, _name_, _attr_ ) \
	tmp = (char*) xmlGetProp( node, (const xmlChar*) _name_ ); \
	_struct_->_attr_ = tmp ? atoi( tmp ) : dflt._attr_; \
	free(tmp)


HistRequest * HistRequest::fromXml( xmlNodePtr node )
{
	char* tmp;
	static const HistRequest dflt;
	
	HistRequest *hR = new HistRequest();
	
	for( xmlNodePtr p = node->children; p!= NULL; p=p->next) {
		if( p->type == XML_ELEMENT_NODE
			&& strcmp((char*)p->name, "reqContract") == 0 )  {
			conv_xml2ib( &hR->_ibContract, p);
		}
	}
	
	GET_ATTR_STRING( hR, "endDateTime", _endDateTime );
	GET_ATTR_STRING( hR, "durationStr", _durationStr );
	GET_ATTR_STRING( hR, "barSizeSetting", _barSizeSetting );
	GET_ATTR_STRING( hR, "whatToShow", _whatToShow );
	GET_ATTR_INT( hR, "useRTH", _useRTH );
	GET_ATTR_INT( hR, "formatDate", _formatDate );
	
	return hR;
}








GenericRequest::GenericRequest() :
	_reqType(NONE),
	_reqId(0),
	_ctime(0)
{
}


GenericRequest::ReqType GenericRequest::reqType() const
{
	return _reqType;
}


int GenericRequest::reqId() const
{
	return _reqId;
}


int GenericRequest::age() const
{
	return (nowInMsecs() - _ctime);
}


void GenericRequest::nextRequest( ReqType t )
{
	_reqType = t;
	_reqId++;
	_ctime = nowInMsecs();
}


void GenericRequest::close()
{
	_reqType = NONE;
}










HistTodo::HistTodo() :
	doneRequests(*(new QList<HistRequest*>())),
	leftRequests(*(new QList<HistRequest*>())),
	errorRequests(*(new QList<HistRequest*>())),
	checkedOutRequest(NULL)
{
}


HistTodo::~HistTodo()
{
	foreach( HistRequest *hR, doneRequests ) {
		delete hR;
	}
	delete &doneRequests;
	foreach( HistRequest *hR, leftRequests ) {
		delete hR;
	}
	delete &leftRequests;
	foreach( HistRequest *hR, errorRequests ) {
		delete hR;
	}
	delete &errorRequests;
	if( checkedOutRequest != NULL ) {
		delete checkedOutRequest;
	}
}


void HistTodo::dumpLeft( FILE *stream ) const
{
	for(int i=0; i < leftRequests.size(); i++ ) {
		fprintf( stream, "[%p]\t%s\n",
		         leftRequests[i],
		         leftRequests[i]->toString().c_str() );
	}
}


int HistTodo::countDone() const
{
	return doneRequests.size();
}


int HistTodo::countLeft() const
{
	return leftRequests.size();
}


void HistTodo::checkout()
{
	assert( checkedOutRequest == NULL );
	checkedOutRequest = leftRequests.takeFirst();
}


int HistTodo::checkoutOpt( PacingGod *pG, const DataFarmStates *dfs )
{
	assert( checkedOutRequest == NULL );
	
	QHash< QString, HistRequest* > hashByFarm;
	QHash<QString, int> countByFarm;
	foreach( HistRequest* hR ,leftRequests ) {
		QString farm = dfs->getHmdsFarm(hR->ibContract());
		if( !hashByFarm.contains(farm) ) {
			hashByFarm.insert(farm, hR);
			countByFarm.insert(farm, 1);
		} else {
			countByFarm[farm]++;
		}
	}
	
	QStringList farms = hashByFarm.keys();
	
	HistRequest *todo_hR = leftRequests.first();
	int countTodo = 0;
	foreach( QString farm, farms ) {
		assert( hashByFarm.contains(farm) );
		 HistRequest *tmp_hR = hashByFarm[farm];
		const IB::Contract& c = tmp_hR->ibContract();
		if( pG->countLeft( c ) > 0 ) {
			if( farm.isEmpty() ) {
				// 1. the unknown ones to learn farm quickly
				todo_hR = tmp_hR;
				break;
			} else if( countTodo < countByFarm[farm] ) {
				// 2. get from them biggest list
				todo_hR = tmp_hR;
				countTodo = countByFarm[farm];
			}
		}
	}
	
	int i = leftRequests.indexOf( todo_hR );
	assert( i>=0 );
	int wait = pG->goodTime( todo_hR->ibContract() );
	if( wait <= 0 ) {
		leftRequests.removeAt( i );
		checkedOutRequest = todo_hR;
	}
	
	return wait;
}


void HistTodo::cancelForRepeat( int priority )
{
	assert( checkedOutRequest != NULL );
	if( priority <= 0 ) {
		leftRequests.prepend(checkedOutRequest);
	} else if( priority <=1 ) {
		leftRequests.append(checkedOutRequest);
	} else {
		errorRequests.append(checkedOutRequest);
	}
	checkedOutRequest = NULL;
}


const HistRequest& HistTodo::current() const
{
	assert( checkedOutRequest != NULL );
	return *checkedOutRequest;
}


void HistTodo::tellDone()
{
	assert( checkedOutRequest != NULL );
	doneRequests.append(checkedOutRequest);
	checkedOutRequest = NULL;
}


void HistTodo::add( const HistRequest& hR )
{
	HistRequest *p = new HistRequest(hR);
	leftRequests.append(p);
}








ContractDetailsTodo::ContractDetailsTodo() :
	contractDetailsRequests(*(new std::vector<ContractDetailsRequest>()))
{
}

ContractDetailsTodo::~ContractDetailsTodo()
{
	delete &contractDetailsRequests;
}








WorkTodo::WorkTodo() :
	reqType(GenericRequest::NONE),
	_contractDetailsTodo( new ContractDetailsTodo() ),
	_histTodo( new HistTodo() )
{
}


WorkTodo::~WorkTodo()
{
	if( _histTodo != NULL ) {
		delete _histTodo;
	}
	if( _contractDetailsTodo != NULL ) {
		delete _contractDetailsTodo;
	}
}


GenericRequest::ReqType WorkTodo::getType() const
{
	return reqType;
}

ContractDetailsTodo* WorkTodo::contractDetailsTodo() const
{
	return _contractDetailsTodo;
}

const ContractDetailsTodo& WorkTodo::getContractDetailsTodo() const
{
	return *_contractDetailsTodo;
}

HistTodo* WorkTodo::histTodo() const
{
	return _histTodo;
}

const HistTodo& WorkTodo::getHistTodo() const
{
	return *_histTodo;
}


int WorkTodo::read_file( const std::string & fileName )
{
	int retVal = -1;
	reqType = GenericRequest::NONE;
	
	TwsXml file;
	if( ! file.openFile(fileName.c_str()) ) {
		return retVal;
	}
	retVal = 0;
	xmlNodePtr xn;
	while( (xn = file.nextXmlNode()) != NULL ) {
		assert( xn->type == XML_ELEMENT_NODE  );
		if( strcmp((char*)xn->name, "request") == 0 ) {
			char* tmp = (char*) xmlGetProp( xn, (const xmlChar*) "type" );
			if( tmp == NULL ) {
				fprintf(stderr, "Warning, no request type specified.\n");
			} else if( strcmp( tmp, "contract_details") == 0 ) {
				reqType = GenericRequest::CONTRACT_DETAILS_REQUEST;
				PacketContractDetails *pcd = PacketContractDetails::fromXml(xn);
				_contractDetailsTodo->contractDetailsRequests.push_back(pcd->getRequest());
				retVal++;
			} else if ( strcmp( tmp, "historical_data") == 0 ) {
				reqType = GenericRequest::HIST_REQUEST;
				PacketHistData *phd = PacketHistData::fromXml(xn);
				_histTodo->add( phd->getRequest() );
				retVal++;
			} else {
				fprintf(stderr, "Warning, unknown request type '%s' ignored.\n",
					tmp );
			}
			free(tmp);
		} else {
			fprintf(stderr, "Warning, unknown request tag '%s' ignored.\n",
				xn->name );
		}
	}
	
	return retVal;
}








PacketContractDetails::PacketContractDetails() :
	cdList(new std::vector<IB::ContractDetails>())
{
	complete = false;
	reqId = -1;
	request = NULL;
}

PacketContractDetails::~PacketContractDetails()
{
	delete cdList;
	if( request != NULL ) {
		delete request;
	}
}

PacketContractDetails * PacketContractDetails::fromXml( xmlNodePtr root )
{
	PacketContractDetails *pcd = new PacketContractDetails();
	
	for( xmlNodePtr p = root->children; p!= NULL; p=p->next) {
		if( p->type == XML_ELEMENT_NODE ) {
			if( strcmp((char*)p->name, "query") == 0 ) {
				pcd->request = ContractDetailsRequest::fromXml(p);
			}
			if( strcmp((char*)p->name, "response") == 0 ) {
				for( xmlNodePtr q = p->children; q!= NULL; q=q->next) {
					if( q->type == XML_ELEMENT_NODE
						&& strcmp((char*)q->name, "ContractDetails") == 0 )  {
						IB::ContractDetails cd;
						conv_xml2ib(&cd, q);
						pcd->cdList->push_back(cd);
					}
				}
			}
		}
	}
	return pcd;
}

const ContractDetailsRequest& PacketContractDetails::getRequest() const
{
	return *request;
}

void PacketContractDetails::record( int reqId,
	const ContractDetailsRequest& cdr )
{
	assert( !complete && this->reqId == -1 && request == NULL
		&& cdList->empty() );
	this->reqId = reqId;
	this->request = new ContractDetailsRequest( cdr );
}

void PacketContractDetails::setFinished()
{
	assert( !complete );
	complete = true;
}


bool PacketContractDetails::isFinished() const
{
	return complete;
}


void PacketContractDetails::clear()
{
	complete = false;
	reqId = -1;
	cdList->clear();
	if( request != NULL ) {
		delete request;
		request = NULL;
	}
}


void PacketContractDetails::append( int reqId, const IB::ContractDetails& c )
{
	if( cdList->empty() ) {
		this->reqId = reqId;
	}
	assert( this->reqId == reqId );
	assert( !complete );
	
	cdList->push_back(c);
}


void PacketContractDetails::dumpXml()
{
	xmlNodePtr root = TwsXml::newDocRoot();
	xmlNodePtr npcd = xmlNewChild( root, NULL,
		(const xmlChar*)"request", NULL );
	xmlNewProp( npcd, (const xmlChar*)"type",
		(const xmlChar*)"contract_details" );
	
	xmlNodePtr nqry = xmlNewChild( npcd, NULL, (xmlChar*)"query", NULL);
	conv_ib2xml( nqry, "reqContract", request->ibContract() );
	
	xmlNodePtr nrsp = xmlNewChild( npcd, NULL, (xmlChar*)"response", NULL);
	for( size_t i=0; i<cdList->size(); i++ ) {
		conv_ib2xml( nrsp, "ContractDetails", (*cdList)[i] );
	}
	
	TwsXml::dumpAndFree( root );
}








void PacketHistData::Row::clear()
{
	date = "";
	open = 0.0;
	high = 0.0;
	low = 0.0;
	close = 0.0;
	volume = 0;
	count = 0;
	WAP = 0.0;
	hasGaps = false;
}

PacketHistData::Row * PacketHistData::Row::fromXml( xmlNodePtr node )
{
	char* tmp;
	static const Row dflt = {"", -1.0, -1.0, -1.0, -1.0, -1, -1, -1.0, 0 };
	Row *row = new Row();
	
	GET_ATTR_STRING( row, "date", date );
	GET_ATTR_DOUBLE( row, "open", open );
	GET_ATTR_DOUBLE( row, "high", high );
	GET_ATTR_DOUBLE( row, "low", low );
	GET_ATTR_DOUBLE( row, "close", close );
	GET_ATTR_INT( row, "volume", volume );
	GET_ATTR_INT( row, "count", count );
	GET_ATTR_DOUBLE( row, "WAP", WAP );
	GET_ATTR_BOOL( row, "hasGaps", hasGaps );
	
	return row;
}


PacketHistData::PacketHistData() :
		rows(*(new QList<Row>()))
{
	mode = CLEAN;
	error = ERR_NONE;
	reqId = -1;
	request = NULL;
}

PacketHistData::~PacketHistData()
{
	delete &rows;
	if( request != NULL ) {
		delete request;
	}
}

PacketHistData * PacketHistData::fromXml( xmlNodePtr root )
{
	PacketHistData *phd = new PacketHistData();
	
	for( xmlNodePtr p = root->children; p!= NULL; p=p->next) {
		if( p->type == XML_ELEMENT_NODE ) {
			if( strcmp((char*)p->name, "query") == 0 ) {
				phd->request = HistRequest::fromXml(p);
			}
			if( strcmp((char*)p->name, "response") == 0 ) {
				for( xmlNodePtr q = p->children; q!= NULL; q=q->next) {
					if( q->type != XML_ELEMENT_NODE ) {
						continue;
					}
					if( strcmp((char*)q->name, "row") == 0 ) {
						Row *row = Row::fromXml( q );
						phd->rows.append(*row);
						delete row;
					} else if( strcmp((char*)q->name, "fin") == 0 ) {
						Row *fin = Row::fromXml( q );
						phd->finishRow = *fin;
						phd->mode = CLOSED;
						delete fin;
					}
				}
			}
		}
	}
	
	return phd;
}


#define ADD_ATTR_STRING( _ne_, _struct_, _attr_ ) \
	if( !TwsXml::skip_defaults || _struct_._attr_ != dflt._attr_ ) { \
		xmlNewProp ( _ne_, (xmlChar*) #_attr_, \
			(xmlChar*) _struct_._attr_.c_str() ); \
	}

#define ADD_ATTR_INT( _ne_, _struct_, _attr_ ) \
	if( !TwsXml::skip_defaults || _struct_._attr_ != dflt._attr_ ) { \
		snprintf(tmp, sizeof(tmp), "%d",_struct_._attr_ ); \
		xmlNewProp ( _ne_, (xmlChar*) #_attr_, (xmlChar*) tmp ); \
	}

#define ADD_ATTR_DOUBLE( _ne_, _struct_, _attr_ ) \
	if( !TwsXml::skip_defaults || _struct_._attr_ != dflt._attr_ ) { \
		snprintf(tmp, sizeof(tmp), "%.10g", _struct_._attr_ ); \
		xmlNewProp ( _ne_, (xmlChar*) #_attr_, (xmlChar*) tmp ); \
	}

#define ADD_ATTR_BOOL( _ne_, _struct_, _attr_ ) \
	if( !TwsXml::skip_defaults || _struct_._attr_ != dflt._attr_ ) { \
		xmlNewProp ( _ne_, (xmlChar*) #_attr_, \
			(xmlChar*) (_struct_._attr_ ? "1" : "0") ); \
	}

void PacketHistData::dumpXml()
{
	char tmp[128];
	
	xmlNodePtr root = TwsXml::newDocRoot();
	xmlNodePtr nphd = xmlNewChild( root, NULL,
		(const xmlChar*)"request", NULL );
	xmlNewProp( nphd, (const xmlChar*)"type",
		(const xmlChar*)"historical_data" );
	
	{
		struct s_bla {
			const std::string endDateTime;
			const std::string durationStr;
			const std::string barSizeSetting;
			const std::string whatToShow;
			int useRTH;
			int formatDate;
		};
		static const s_bla dflt = {"", "", "", "", 0, 0 };
		const IB::Contract &c = request->ibContract();
		s_bla bla = { request->endDateTime(), request->durationStr(),
			request->barSizeSetting(), request->whatToShow(), request->useRTH(),
			request->formatDate() };
		
		xmlNodePtr nqry = xmlNewChild( nphd, NULL, (xmlChar*)"query", NULL);
		conv_ib2xml( nqry, "reqContract", c );
		ADD_ATTR_STRING( nqry, bla, endDateTime );
		ADD_ATTR_STRING( nqry, bla, durationStr );
		ADD_ATTR_STRING( nqry, bla, barSizeSetting );
		ADD_ATTR_STRING( nqry, bla, whatToShow );
		ADD_ATTR_INT( nqry, bla, useRTH );
		ADD_ATTR_INT( nqry, bla, formatDate );
	}
	
	if( mode == CLOSED ) {
	xmlNodePtr nrsp = xmlNewChild( nphd, NULL, (xmlChar*)"response", NULL);
	{
		static const Row dflt = {"", -1.0, -1.0, -1.0, -1.0, -1, -1, -1.0, 0 };
		for( int i=0; i<rows.size(); i++ ) {
			xmlNodePtr nrow = xmlNewChild( nrsp, NULL, (xmlChar*)"row", NULL);
			ADD_ATTR_STRING( nrow, rows[i], date );
			ADD_ATTR_DOUBLE( nrow, rows[i], open );
			ADD_ATTR_DOUBLE( nrow, rows[i], high );
			ADD_ATTR_DOUBLE( nrow, rows[i], low );
			ADD_ATTR_DOUBLE( nrow, rows[i], close );
			ADD_ATTR_INT( nrow, rows[i], volume );
			ADD_ATTR_INT( nrow, rows[i], count );
			ADD_ATTR_DOUBLE( nrow, rows[i], WAP );
			ADD_ATTR_BOOL( nrow, rows[i], hasGaps );
		}
		xmlNodePtr nrow = xmlNewChild( nrsp, NULL, (xmlChar*)"fin", NULL);
		ADD_ATTR_STRING( nrow, finishRow, date );
		ADD_ATTR_DOUBLE( nrow, finishRow, open );
		ADD_ATTR_DOUBLE( nrow, finishRow, high );
		ADD_ATTR_DOUBLE( nrow, finishRow, low );
		ADD_ATTR_DOUBLE( nrow, finishRow, close );
		ADD_ATTR_INT( nrow, finishRow, volume );
		ADD_ATTR_INT( nrow, finishRow, count );
		ADD_ATTR_DOUBLE( nrow, finishRow, WAP );
		ADD_ATTR_BOOL( nrow, finishRow, hasGaps );
	}
	}
	TwsXml::dumpAndFree( root );
}

const HistRequest& PacketHistData::getRequest() const
{
	return *request;
}

bool PacketHistData::isFinished() const
{
	return (mode == CLOSED);
}


PacketHistData::Error PacketHistData::getError() const
{
	return error;
}


void PacketHistData::clear()
{
	mode = CLEAN;
	error = ERR_NONE;
	reqId = -1;
	if( request != NULL ) {
		delete request;
		request = NULL;
	}
	rows.clear();
	finishRow.clear();
}


void PacketHistData::record( int reqId, const HistRequest& hR )
{
	assert( mode == CLEAN && error == ERR_NONE && request == NULL );
	mode = RECORD;
	this->reqId = reqId;
	this->request = new HistRequest( hR );
}


void PacketHistData::append( int reqId, const std::string &date,
			double open, double high, double low, double close,
			int volume, int count, double WAP, bool hasGaps )
{
	assert( mode == RECORD && error == ERR_NONE );
	assert( this->reqId == reqId );
	
	Row row = { date, open, high, low, close,
		volume, count, WAP, hasGaps };
	
	if( strncmp(date.c_str(), "finished", 8) == 0) {
		mode = CLOSED;
		finishRow = row;
	} else {
		rows.append( row );
	}
}


void PacketHistData::closeError( Error e )
{
	assert( mode == RECORD);
	mode = CLOSED;
	error = e;
}


void PacketHistData::dump( bool printFormatDates )
{
	assert( mode == CLOSED && error == ERR_NONE );
	const IB::Contract &c = request->ibContract();
	const char *wts = short_wts( request->whatToShow().c_str() );
	const char *bss = short_bar_size( request->barSizeSetting().c_str());
	
	foreach( Row r, rows ) {
		std::string expiry = c.expiry;
		std::string dateTime = r.date;
		if( printFormatDates ) {
			if( expiry.empty() ) {
				expiry = "0000-00-00";
			} else {
				expiry = ibDate2ISO( c.expiry );
			}
			dateTime = ibDate2ISO(r.date);
			assert( !expiry.empty() && !dateTime.empty() ); //TODO
		}
		
		char buf_c[512];
		snprintf( buf_c, sizeof(buf_c), "%s\t%s\t%s\t%s\t%s\t%g\t%s",
			c.symbol.c_str(),
			c.secType.c_str(),
			c.exchange.c_str(),
			c.currency.c_str(),
			expiry.c_str(),
			c.strike,
			c.right.c_str() );
		
		printf("%s\t%s\t%s\t%s\t%f\t%f\t%f\t%f\t%d\t%d\t%f\t%d\n",
		       wts,
		       bss,
		       buf_c,
		       dateTime.c_str(),
		       r.open, r.high, r.low, r.close,
		       r.volume, r.count, r.WAP, r.hasGaps);
		fflush(stdout);
	}
}








PacingControl::PacingControl( int r, int i, int m, int v ) :
	dateTimes(*(new QList<int64_t>())),
	violations(*(new QList<bool>())),
	maxRequests( r ),
	checkInterval( i ),
	minPacingTime( m ),
	violationPause( v )
{
}

PacingControl::~PacingControl()
{
	delete &violations;
	delete &dateTimes;
}

void PacingControl::setPacingTime( int r, int i, int m )
{
	maxRequests = r;
	checkInterval = i;
	minPacingTime = m;
}


void PacingControl::setViolationPause( int vP )
{
	violationPause = vP;
}


bool PacingControl::isEmpty() const
{
	return dateTimes.isEmpty();
}


void PacingControl::clear()
{
	if( !dateTimes.isEmpty() ) {
		int64_t now = nowInMsecs();
		if( now - dateTimes.last() < 5000  ) {
			// HACK race condition might cause assert in notifyViolation(),
			// to avoid this we would need to ack each request
			DEBUG_PRINTF( "Warning, keep last pacing date time "
				"because it looks too new." );
			dateTimes.erase( dateTimes.begin(), --(dateTimes.end()) );
			violations.erase( violations.begin(), --(violations.end()) );
		} else {
			dateTimes.clear();
			violations.clear();
		}
	}
}


void PacingControl::addRequest()
{
	const int64_t now_t = nowInMsecs();
	dateTimes.append( now_t );
	violations.append( false );
}


void PacingControl::notifyViolation()
{
	assert( !violations.isEmpty() );
	violations.last() = true;
}


#define SWAP_MAX( _waitX_, _dbg_ ) \
	if( retVal < _waitX_ ) { \
		retVal = _waitX_; \
		dbg = _dbg_ ; \
	}

int PacingControl::goodTime(const char** ddd) const
{
	const int64_t now = nowInMsecs();
	const char* dbg = "don't wait";
	int retVal = INT_MIN;
	
	if( dateTimes.isEmpty() ) {
		*ddd = dbg;
		return retVal;
	}
	
	
	int waitMin = dateTimes.last() + minPacingTime - now;
	SWAP_MAX( waitMin, "wait min" );
	
// 	int waitAvg =  dateTimes.last() + avgPacingTime - now;
// 	SWAP_MAX( waitAvg, "wait avg" );
	
	int waitViol = violations.last() ?
		(dateTimes.last() + violationPause - now) : INT_MIN;
	SWAP_MAX( waitViol, "wait violation" );
	
	int waitBurst = INT_MIN;
	int p_index = dateTimes.size() - maxRequests;
	if( p_index >= 0 ) {
		int64_t p_time = dateTimes.at( p_index );
		waitBurst = p_time + checkInterval - now;
	}
	SWAP_MAX( waitBurst, "wait burst" );
	
	*ddd = dbg;
	return retVal;
}

#undef SWAP


int PacingControl::countLeft() const
{
	const int64_t now = nowInMsecs();
	
	if( (dateTimes.size() > 0) && violations.last() ) {
		int waitViol = dateTimes.last() + violationPause - now;
		if( waitViol > 0 ) {
			return 0;
		}
	}
	
	int retVal = maxRequests;
	QList<int64_t>::const_iterator it = dateTimes.constEnd();
	while( it != dateTimes.constBegin() ) {
		it--;
		int waitBurst = *it + checkInterval - now;
		if( waitBurst > 0 ) {
			retVal--;
		} else {
			break;
		}
	}
	return retVal;
}


void PacingControl::merge( const PacingControl& other )
{
// 	qDebug() << dateTimes;
// 	qDebug() << other.dateTimes;
	QList<int64_t>::iterator t_d = dateTimes.begin();
	QList<bool>::iterator t_v = violations.begin();
	QList<int64_t>::const_iterator o_d = other.dateTimes.constBegin();
	QList<bool>::const_iterator o_v = other.violations.constBegin();
	
	while( t_d != dateTimes.end() && o_d != other.dateTimes.constEnd() ) {
		if( *o_d < *t_d ) {
			t_d = dateTimes.insert( t_d, *o_d );
			t_v = violations.insert( t_v, *o_v );
			o_d++;
			o_v++;
		} else {
			t_d++;
			t_v++;
		}
	}
	while( o_d != other.dateTimes.constEnd() ) {
		assert( t_d == dateTimes.end() );
		t_d = dateTimes.insert( t_d, *o_d );
		t_v = violations.insert( t_v, *o_v );
		t_d++;
		t_v++;
		o_d++;
		o_v++;
	}
// 	qDebug() << dateTimes;
}








#define LAZY_CONTRACT_STR( _c_ ) \
	std::string(_c_.exchange).append("\t").append(_c_.secType)


PacingGod::PacingGod( const DataFarmStates &dfs ) :
	dataFarms( dfs ),
	maxRequests( 60 ),
	checkInterval( 601000 ),
	minPacingTime( 1500 ),
	violationPause( 60000 ),
	controlGlobal( *(new PacingControl(
		maxRequests, checkInterval, minPacingTime, violationPause)) ),
	controlHmds(*(new QHash<const QString, PacingControl*>()) ),
	controlLazy(*(new QHash<const QString, PacingControl*>()) )
{
}


PacingGod::~PacingGod()
{
	delete &controlGlobal;
	
	QHash<const QString, PacingControl*>::iterator it;
	it = controlHmds.begin();
	while( it != controlHmds.end() ) {
		delete *it;
		it = controlHmds.erase(it);
	}
	it = controlLazy.begin();
	while( it != controlLazy.end() ) {
		delete *it;
		it = controlLazy.erase(it);
	}
	delete &controlHmds;
	delete &controlLazy;
}


void PacingGod::setPacingTime( int r, int i, int m )
{
	maxRequests = r;
	checkInterval = i;
	minPacingTime = m;
	controlGlobal.setPacingTime( r, i, m );
	foreach( PacingControl *pC, controlHmds ) {
		pC->setPacingTime( r, i, m );
	}
	foreach( PacingControl *pC, controlLazy ) {
		pC->setPacingTime( r, i, m  );
	}
}


void PacingGod::setViolationPause( int vP )
{
	violationPause = vP;
	controlGlobal.setViolationPause( vP );
	foreach( PacingControl *pC, controlHmds ) {
		pC->setViolationPause( vP );
	}
	foreach( PacingControl *pC, controlLazy ) {
		pC->setViolationPause( vP );
	}
}


void PacingGod::clear()
{
	if( dataFarms.getActives().isEmpty() ) {
		// clear all PacingControls
		DEBUG_PRINTF( "clear all pacing controls" );
		controlGlobal.clear();
		foreach( PacingControl *pC, controlHmds ) {
			pC->clear();
		}
		foreach( PacingControl *pC, controlLazy ) {
			pC->clear();
		}
	} else {
		// clear only PacingControls of inactive farms
		foreach( QString farm, dataFarms.getInactives() ) {
			if( controlHmds.contains(farm) ) {
				DEBUG_PRINTF( "clear pacing control of inactive farm %s",
					farm.toStdString().c_str() );
				controlHmds.value(farm)->clear();
			}
		}
	}
}


void PacingGod::addRequest( const IB::Contract& c )
{
	QString farm;
	QString lazyC;
	checkAdd( c, &lazyC, &farm );
	
	controlGlobal.addRequest();
	
	if( farm.isEmpty() ) {
		DEBUG_PRINTF( "add request lazy" );
		assert( controlLazy.contains(lazyC) && !controlHmds.contains(farm) );
		controlLazy[lazyC]->addRequest();
	} else {
		DEBUG_PRINTF( "add request farm %s", farm.toStdString().c_str() );
		assert( controlHmds.contains(farm) && !controlLazy.contains(lazyC) );
		controlHmds[farm]->addRequest();
	}
}


void PacingGod::notifyViolation( const IB::Contract& c )
{
	QString farm;
	QString lazyC;
	checkAdd( c, &lazyC, &farm );
	
	controlGlobal.notifyViolation();
	
	if( farm.isEmpty() ) {
		DEBUG_PRINTF( "set violation lazy" );
		assert( controlLazy.contains(lazyC) && !controlHmds.contains(farm) );
		controlLazy[lazyC]->notifyViolation();
	} else {
		DEBUG_PRINTF( "set violation farm %s", farm.toStdString().c_str() );
		assert( controlHmds.contains(farm) && !controlLazy.contains(lazyC) );
		controlHmds[farm]->notifyViolation();
	}
}


int PacingGod::goodTime( const IB::Contract& c )
{
	const char* dbg;
	QString farm;
	QString lazyC;
	checkAdd( c, &lazyC, &farm );
	bool laziesCleared = laziesAreCleared();
	
	if( farm.isEmpty() || !laziesCleared ) {
		// we have to use controlGlobal if any contract's farm is ambiguous
		assert( (controlLazy.contains(lazyC) && !controlHmds.contains(farm))
			|| !laziesCleared );
		int t = controlGlobal.goodTime(&dbg);
		DEBUG_PRINTF( "get good time global %s %d", dbg, t );
		return t;
	} else {
		assert( (controlHmds.contains(farm) && controlLazy.isEmpty())
			|| laziesCleared );
		int t = controlHmds.value(farm)->goodTime(&dbg);
		DEBUG_PRINTF( "get good time farm %s %s %d",
			farm.toStdString().c_str(), dbg, t );
		return t;
	}
}


int PacingGod::countLeft( const IB::Contract& c )
{
	QString farm;
	QString lazyC;
	checkAdd( c, &lazyC, &farm );
	bool laziesCleared = laziesAreCleared();
	
	if( farm.isEmpty() || !laziesCleared ) {
		// we have to use controlGlobal if any contract's farm is ambiguous
		assert( (controlLazy.contains(lazyC) && !controlHmds.contains(farm))
			|| !laziesCleared );
		int left = controlGlobal.countLeft();
		DEBUG_PRINTF( "get count left global %d", left );
		return left;
	} else {
		assert( (controlHmds.contains(farm) && controlLazy.isEmpty())
			|| laziesCleared );
		int left = controlHmds.value(farm)->countLeft();
		DEBUG_PRINTF( "get count left farm %s %d",
			farm.toStdString().c_str(), left );
		return controlHmds.value(farm)->countLeft();
	}
}



void PacingGod::checkAdd( const IB::Contract& c,
	QString *lazyC_, QString *farm_ )
{
	*lazyC_ = QString::fromStdString(LAZY_CONTRACT_STR(c));
	*farm_ = dataFarms.getHmdsFarm(c);
	
	// controlLazy.keys() does not work for QHash<const QString, PacingControl*>
	QStringList lazies;
	QHash<const QString, PacingControl*>::const_iterator it =
		controlLazy.constBegin();
	while( it != controlLazy.constEnd() ) {
		lazies.append( it.key() );
		it++;
	}
	if( !lazies.contains(*lazyC_) ) {
		lazies.append(*lazyC_);
	}
	
	foreach( QString lazyC, lazies ) {
	QString farm = dataFarms.getHmdsFarm(lazyC);
	if( !farm.isEmpty() ) {
		if( !controlHmds.contains(farm) ) {
			PacingControl *pC;
			if( controlLazy.contains(lazyC) ) {
				DEBUG_PRINTF( "move pacing control lazy to farm %s, %s",
					lazyC.toStdString().c_str(), farm.toStdString().c_str() );
				pC = controlLazy.take(lazyC);
			} else {
				DEBUG_PRINTF( "create pacing control for farm %s",
					farm.toStdString().c_str() );
				pC = new PacingControl(
					maxRequests, checkInterval, minPacingTime, violationPause);
			}
			controlHmds.insert( farm, pC );
		} else {
			if( !controlLazy.contains(lazyC) ) {
				// fine - no history about that
			} else {
				DEBUG_PRINTF( "merge pacing control lazy into farm %s %s",
					lazyC.toStdString().c_str(), farm.toStdString().c_str() );
				PacingControl *pC = controlLazy.take(lazyC);
				controlHmds.value(farm)->merge(*pC);
				delete pC;
			}
		}
		assert( controlHmds.contains(farm) );
		assert( !controlLazy.contains(lazyC) );
		
	} else if( !controlLazy.contains(lazyC) ) {
			DEBUG_PRINTF( "create pacing control for lazy %s",
				lazyC.toStdString().c_str() );
			PacingControl *pC = new PacingControl(
				maxRequests, checkInterval, minPacingTime, violationPause);
			controlLazy.insert( lazyC, pC );
			
			assert( !controlHmds.contains(farm) );
			assert( controlLazy.contains(lazyC) );
	}
	}
}


bool PacingGod::laziesAreCleared() const
{
	
	bool retVal = true;
	foreach( PacingControl *pC, controlLazy ) {
		retVal &=  pC->isEmpty();
	}
	return retVal;
}








DataFarmStates::DataFarmStates() :
	mStates( *(new QHash<const QString, State>()) ),
	hStates( *(new QHash<const QString, State>()) ),
	mLearn( *(new QHash<const QString, QString>()) ),
	hLearn( *(new QHash<const QString, QString>()) ),
	lastMsgNumber(INT_MIN)
{
	initHardCodedFarms();
}

DataFarmStates::~DataFarmStates()
{
	delete &hLearn;
	delete &mLearn;
	delete &hStates;
	delete &mStates;
}

void DataFarmStates::initHardCodedFarms()
{
	hLearn["BELFOX\tIND"] = "euhmds2";
	hLearn["BM\tIND"] = "euhmds2";
	hLearn["BVME\tIND"] = "euhmds2";
	hLearn["CBOT\tIND"] = "euhmds2";
	hLearn["DTB\tIND"] = "euhmds2";
	hLearn["EBS\tIND"] = "euhmds2";
	hLearn["EDXNO\tIND"] = "euhmds2";
	hLearn["FTA\tIND"] = "euhmds2";
	hLearn["IBIS\tIND"] = "euhmds2";
	hLearn["IDEM\tIND"] = "euhmds2";
	hLearn["IPE\tIND"] = "euhmds2";
	hLearn["LIFFE_NF\tIND"] = "euhmds2";
	hLearn["LIFFE\tIND"] = "euhmds2";
	hLearn["LSE\tIND"] = "euhmds2";
	hLearn["MATIF\tIND"] = "euhmds2";
	hLearn["MEFFRV\tIND"] = "euhmds2";
	hLearn["MONEP\tIND"] = "euhmds2";
	hLearn["OMS\tIND"] = "euhmds2";
	hLearn["SFB\tIND"] = "euhmds2";
	hLearn["SOFFEX\tIND"] = "euhmds2";
	
	hLearn["BELFOX\tFUT"] = "euhmds2";
	hLearn["DTB\tFUT"] = "euhmds2";
	hLearn["FTA\tFUT"] = "euhmds2";
	hLearn["LIFFE_NF\tFUT"] = "euhmds2";
	hLearn["LIFFE\tFUT"] = "euhmds2";
	hLearn["LSSF\tFUT"] = "euhmds2";
	hLearn["MONEP\tFUT"] = "euhmds2";
	hLearn["SOFFEX\tFUT"] = "euhmds2";
	
	hLearn["DTB\tOPT"] = "euhmds2";
	hLearn["LIFFE\tOPT"] = "euhmds2";
	hLearn["MONEP\tOPT"] = "euhmds2";
	
	
	hLearn["IDEALPRO\tCASH"] = "ushmds2a";
	
	hLearn["AMEX\tIND"] = "ushmds2a";
	hLearn["AQS\tIND"] = "ushmds2a";
	hLearn["ARCA\tIND"] = "ushmds2a";
	hLearn["CBOE\tIND"] = "ushmds2a";
	hLearn["CDE\tIND"] = "ushmds2a";
	hLearn["CFE\tIND"] = "ushmds2a";
	hLearn["CME\tIND"] = "ushmds2a";
	hLearn["ECBOT\tIND"] = "ushmds2a";
	hLearn["GLOBEX\tIND"] = "ushmds2a";
	hLearn["ISE\tIND"] = "ushmds2a";
	hLearn["NASDAQ\tIND"] = "ushmds2a";
	hLearn["NYBOT\tIND"] = "ushmds2a";
	hLearn["NYMEX\tIND"] = "ushmds2a";
	hLearn["NYSELIFFE\tIND"] = "ushmds2a";
	hLearn["NYSE\tIND"] = "ushmds2a";
	hLearn["ONE\tIND"] = "ushmds2a";
	hLearn["PHLX\tIND"] = "ushmds2a";
	hLearn["PSE\tIND"] = "ushmds2a";
	hLearn["TSE\tIND"] = "ushmds2a";
	
	hLearn["CDE\tFUT"] = "ushmds2a";
	hLearn["CFE\tFUT"] = "ushmds2a";
	hLearn["CME\tFUT"] = "ushmds2a";
	hLearn["ECBOT\tFUT"] = "ushmds2a";
	hLearn["GLOBEX\tFUT"] = "ushmds2a";
	hLearn["IPE\tFUT"] = "ushmds2a";
	hLearn["NYBOT\tFUT"] = "ushmds2a";
	hLearn["NYMEX\tFUT"] = "ushmds2a";
	hLearn["NYSELIFFE\tFUT"] = "ushmds2a";
	hLearn["ONE\tFUT"] = "ushmds2a";
	
	hLearn["CBOE\tOPT"] = "ushmds2a";
	
	
	hLearn["ASX\tIND"] = "hkhmds2";
	hLearn["HKFE\tIND"] = "hkhmds2";
	hLearn["KSE\tIND"] = "hkhmds2";
	hLearn["NSE\tIND"] = "hkhmds2";
	hLearn["OSE.JPN\tIND"] = "hkhmds2";
	hLearn["SGX\tIND"] = "hkhmds2";
	hLearn["SNFE\tIND"] = "hkhmds2";
	hLearn["TSE.JPN\tIND"] = "hkhmds2";
	
	hLearn["HKFE\tFUT"] = "hkhmds2";
	hLearn["NSE\tFUT"] = "hkhmds2";
	hLearn["SGX\tFUT"] = "hkhmds2";
	hLearn["SNFE\tFUT"] = "hkhmds2";
	
	hLearn["KSE\tOPT"] = "hkhmds2";
}

void DataFarmStates::setAllBroken()
{
	QHash<const QString, State>::iterator it;
	
	it = mStates.begin();
	while( it != mStates.end() ) {
		*it = BROKEN;
		it++;
	}
	
	it = hStates.begin();
	while( it != hStates.end() ) {
		*it = BROKEN;
		it++;
	}
}


void DataFarmStates::notify(int msgNumber, int errorCode,
	const std::string &_msg)
{
	QString msg = QString::fromStdString(_msg); // just convert to QString
	lastMsgNumber = msgNumber;
	QString farm;
	State state;
	QHash<const QString, State> *pHash = NULL;
	
	// prefixes used for getFarm() are taken from past logs
	switch( errorCode ) {
	case 2103:
		//API docu: "A market data farm is disconnected."
		pHash = &mStates;
		state = BROKEN;
		farm = getFarm("Market data farm connection is broken:", msg);
		break;
	case 2104:
		//API docu: "A market data farm is connected."
		pHash = &mStates;
		state = OK;
		farm = getFarm("Market data farm connection is OK:", msg);
		break;
	case 2105:
		//API docu: "A historical data farm is disconnected."
		pHash = &hStates;
		state = BROKEN;
		farm = getFarm("HMDS data farm connection is broken:", msg);
		break;
	case 2106:
		//API docu: "A historical data farm is connected."
		pHash = &hStates;
		state = OK;
		farm = getFarm("HMDS data farm connection is OK:", msg);
		break;
	case 2107:
		//API docu: "A historical data farm connection has become inactive but should be available upon demand."
		pHash = &hStates;
		state = INACTIVE;
		farm = getFarm("HMDS data farm connection is inactive but should be available upon demand.", msg);
		break;
	case 2108:
		//API docu: "A market data farm connection has become inactive but should be available upon demand."
		pHash = &mStates;
		state = INACTIVE;
		farm = getFarm("Market data farm connection is inactive but should be available upon demand.", msg);
		break;
	default:
		assert(false);
		return;
	}
	
	lastChanged = farm.toStdString();
	pHash->insert( farm, state );
// 	qDebug() << *pHash; // TODO print farms with states
}


/// static member
QString DataFarmStates::getFarm( const QString prefix, const QString& msg )
{
	assert( msg.startsWith(prefix, Qt::CaseInsensitive) );
	
	return msg.right( msg.size() - prefix.size() );
}


void DataFarmStates::learnMarket( const IB::Contract& )
{
	assert( false ); //not implemented
}


void DataFarmStates::learnHmds( const IB::Contract& c )
{
	QString lazyC = QString::fromStdString(LAZY_CONTRACT_STR(c));
	
	QStringList sl;
	QHash<const QString, State>::const_iterator it = hStates.constBegin();
	while( it != hStates.constEnd() ) {
		if( *it == OK ) {
			sl.append( it.key() );
		}
		it++;
	}
	if( sl.size() <= 0 ) {
		assert(false); // assuming at least one farm must be active
	} else if( sl.size() == 1 ) {
		if( hLearn.contains(lazyC) ) {
			assert( hLearn.value( lazyC ) == sl.first() );
		} else {
			hLearn.insert( lazyC, sl.first() );
			DEBUG_PRINTF( "learn HMDS farm (unique): %s %s",
				lazyC.toStdString().c_str(), sl.first().toStdString().c_str() );
		}
	} else {
		if( hLearn.contains(lazyC) ) {
			assert( sl.contains(hLearn.value(lazyC)) );
		} else {
			//but doing nothing
			DEBUG_PRINTF( "learn HMDS farm (ambiguous): %s (%s)",
				lazyC.toStdString().c_str(),
				sl.join(",").toStdString().c_str() );
		}
	}
}


void DataFarmStates::learnHmdsLastOk(int msgNumber, const IB::Contract& c )
{
	QString lastChanged = QString::fromStdString(this->lastChanged);
	assert( !lastChanged.isEmpty() && hStates.contains(lastChanged) );
	if( (msgNumber == (lastMsgNumber + 1)) && (hStates[lastChanged] == OK) ) {
		QString lazyC = QString::fromStdString(LAZY_CONTRACT_STR(c));
		if( hLearn.contains(lazyC) ) {
			assert( hLearn.value( lazyC ) == lastChanged );
		} else {
			hLearn.insert( lazyC, lastChanged );
			DEBUG_PRINTF( "learn HMDS farm (last ok): %s %s",
				lazyC.toStdString().c_str(), lastChanged.toStdString().c_str());
		}
	}
}


QStringList DataFarmStates::getInactives() const
{
	QStringList sl;
	QHash<const QString, State>::const_iterator it = hStates.constBegin();
	while( it != hStates.constEnd() ) {
		if( *it == INACTIVE || *it == BROKEN ) {
			sl.append( it.key() );
		}
		it++;
	}
	return sl;
}


QStringList DataFarmStates::getActives() const
{
	QStringList sl;
	QHash<const QString, State>::const_iterator it = hStates.constBegin();
	while( it != hStates.constEnd() ) {
		if( *it == OK ) {
			sl.append( it.key() );
		}
		it++;
	}
	return sl;
}


QString DataFarmStates::getMarketFarm( const IB::Contract& c ) const
{
	QString lazyC = QString::fromStdString(LAZY_CONTRACT_STR(c));
	return mLearn.value(lazyC);
}


QString DataFarmStates::getHmdsFarm( const IB::Contract& c ) const
{
	QString lazyC = QString::fromStdString(LAZY_CONTRACT_STR(c));
	return hLearn.value(lazyC);
}


QString DataFarmStates::getHmdsFarm( const QString& lazyC ) const
{
	return hLearn.value(lazyC);
}


#undef LAZY_CONTRACT_STR




