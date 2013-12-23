
#include <stdio.h>
#include _DMSTIMER_HEADER_

#if (!_DMS_EXTERNALUPDATE_)
	void dms_update(void)
	{}

	void init_dmsExternalUpdate(uint8_t meh, uint8_t bar)
	{}
#endif

int main(void)
{
	uint8_t numUpdates = 70; //3; //5;
	uint8_t incrementSize = 111;//5; //3;


	init_dmsExternalUpdate(numUpdates, incrementSize);


	int i;

	//dms_update();
	dms4day_t startTime = dmsGetTime();

	for(i=0; i<25; i++)
	{
		//dms_update();

		dms4day_t now = dmsGetTime() - startTime;
		printf("%d: dms=%d, dmsFrac=%d\n", i, now, dmsGetFrac());

		dms_update();
	}



	return 0;
}
