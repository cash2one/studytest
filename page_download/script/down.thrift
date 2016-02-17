struct task_arg {
     1:i32 task_id=0,
}

service srpp_task {
	i32 ping(),
	string run_action(1:task_arg arg)
}

