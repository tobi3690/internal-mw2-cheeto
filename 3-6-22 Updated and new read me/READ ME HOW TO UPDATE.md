#How to semi-update from uc offsets and decryptions to this source.

fixes and tricks.
Offsets:
Well aside from me making it pretty much paster friendly directly from uc offsets to the source you should have no issues besides the names.



Decryptions:
usually you will see either driver::read or <uintptr_t> basicalyl driver::read is mostly used for externals but in our case we are internal.
You will need to change these obviously 

You can easily do CNTRL + F to lookup and type in one of the above. and replace with *(uintptr_t*) , you will probably get either mb is not defined, or baseModuleAddr, right under the decryption names you can either do `auto mb = g_data::base; and CNTRL + F lookup baseModuleAddr and click the arrow down and type in mb, click replace all and you should not have any errors.

if you get Peb is not defined do the same thing above but only add auto peb = g_data::peb; remember, you can really get errors thrown at you if you are not using lowercase or uppercase remember this. 

im aware of some people getting mixed up with the names like "Why on your src it says get_client_info and on uc it says decrypt_client_info, well same as offsets not everyones shit is named the same ofc.  simply just change it to get_client_info or the name provided in the src. Also take your time putting these decryptions in, i know people just copy str8 from uc and paste. No you will get stuck in a brainfart and come to uc for help ( which you will be clowned on for ) take your time even if its copying one by one fixing the errors then moving onto the next. Stop rushing. I also reuploaded this src with the fix to the I_MenuTab being undefined not sure why ppl cant look thru the forum but `int I_MenuTab = 0; 0 defines the esp tab, you will automatically load on here when you inject.

If you want me to add more error and fixes i will soon

Thank you for your time. You can contact me on discord @ discord.gg/imgui or my username (will change check my uc signature if it does not work) 
tobiwobi#2242