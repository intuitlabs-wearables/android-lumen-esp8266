/*
 * Copyright (c) 2015 Intuit Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package com.intuitlabs.android.lumenesp8266;

import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceActivity;
import android.os.Bundle;
import android.util.Log;

import com.intuitlabs.android.lumenesp8266.service.RegistrationIntentService;

@SuppressWarnings("deprecation")
public class MainActivity extends PreferenceActivity implements SharedPreferences.OnSharedPreferenceChangeListener {
    private static final String LOG_TAG = MainActivity.class.getName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.preferences);
        startService(new Intent(this, RegistrationIntentService.class));
    }

    /**
     * Register a this instance as a {@link android.content.SharedPreferences.OnSharedPreferenceChangeListener}
     *
     * @see #onPause for unregistering
     */
    @Override
    public void onResume() {
        super.onResume();
        getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
    }

    /**
     * Unregister a this instance as a {@link android.content.SharedPreferences.OnSharedPreferenceChangeListener}
     *
     * @see #onResume for registering
     */
    @Override
    public void onPause() {
        super.onPause();
        getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
    }

    //
    // Implement SharedPreferences.OnSharedPreferenceChangeListener
    //

    /**
     * Take action to sync settings with the Push Notification provider,
     * if the topic selection has changed.
     *
     * @param sharedPreferences {@link android.content.SharedPreferences}
     * @param key               {@link String}
     */
    @Override
    public void onSharedPreferenceChanged(final SharedPreferences sharedPreferences, final String key) {
        if (key.equals(getString(R.string.preference_key_topics))) {
            Log.d(LOG_TAG, "Subscriptions changed");
            sharedPreferences.edit().putBoolean(getString(R.string.preference_key_insync), false).apply();
            startService(new Intent(this, RegistrationIntentService.class));
        }
    }
}
