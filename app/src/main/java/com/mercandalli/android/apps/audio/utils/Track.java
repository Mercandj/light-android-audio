package com.mercandalli.android.apps.audio.utils;

public class Track {

    private long mId;
    private String mArtistName;
    private String mDisplayName;
    private String mTitle;
    private String mPath;

    public Track(
            final long id,
            final String artistName,
            final String displayName,
            final String title,
            final String path) {
        this.mId = id;
        this.mArtistName = artistName;
        this.mDisplayName = displayName;
        this.mTitle = title;
        this.mPath = path;
    }

    public long getId() {
        return mId;
    }

    public String getArtistName() {
        return mArtistName;
    }

    public String getDisplayName() {
        return mDisplayName;
    }

    public String getTitle() {
        return mTitle;
    }

    public String getPath() {
        return mPath;
    }
}
