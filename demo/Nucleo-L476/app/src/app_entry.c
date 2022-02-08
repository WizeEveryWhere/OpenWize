
#ifdef __cplusplus
extern "C" {
#endif

#include "app_entry.h"

extern void Sys_Init(void);
extern void Sys_Start(void);
extern void LoItf_Setup(void);

void App_Init(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
void app_entry(void)
{
  	Sys_Init();
  	App_Init();
  	Sys_Start();
}

/******************************************************************************/
void App_Init(void)
{
	LoItf_Setup();
}

/******************************************************************************/

#ifdef __cplusplus
}
#endif
