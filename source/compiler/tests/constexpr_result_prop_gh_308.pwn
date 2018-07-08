#if (30 < 40 < 50)
	#warning "Test passed."
#endif

#if !(30 < 40 < 35)
	#warning "Test passed."
#endif

#if (30 < 40)
	#warning "Test passed."
#endif

#if !(40 < 35)
	#warning "Test passed."
#endif

main () {

}
