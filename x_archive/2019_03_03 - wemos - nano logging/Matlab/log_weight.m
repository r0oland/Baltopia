

readChannelID = 712441;
fieldID1 = 2;
readAPIKey = '2Q9GKRS5ND01LNF0';
resistance = [];

measRunning = true;
nMeas = 0;

% cam = webcam(2);
% cam.Resolution = '640x480';

while(measRunning)
  nMeas = nMeas+1;


  % create snap shot
  img = snapshot(cam);
  img = rgb2gray(img);
  % img = img(60:158,63:450);
  imName = sprintf('temp_%04.0f.jpg',nMeas);
  imwrite(img,['C:\Users\johan\Pictures\temp_images\' imName]);

  [resistance(end+1),~,channelInfo] = thingSpeakRead(readChannelID,'Field',fieldID1,'ReadKey', readAPIKey,'NumPoints', 1);
  thisID = channelInfo.LastEntryID;

  fprintf('Measuring resistance (%i) and taking picture (%i)\n',resistance(end),nMeas);

  if resistance >= 9999 % stop here
    measRunning = false;
  end

  pause(60*10);
end
