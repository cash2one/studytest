namespace py srpp_parser

struct parse_arg {
     1:string html_content="",
	 2:string extra="",
	 3:string engine="sogou",
	 4:i32 ret_action=0, 
}

service srpp_parser {
	i32 ping(),
	string parseHtml(1:parse_arg arg)
}
