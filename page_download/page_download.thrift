namespace cpp page_download

struct crawler_arg {
     1:i32 task_id=0,
	 2:i32 start=0,
	 3:i32 limit=0,
}

service grab_page {
	i32 ping(),
	i32 start_grab(1:crawler_arg arg)
}
